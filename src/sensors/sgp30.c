/*
 * src/sensors/sgp30.c
 *
 * Custom SGP30 driver using raw Zephyr I2C API.
 *
 * Zephyr 4.x does not ship a built-in SGP30 sensor driver (only SGP40).
 * We implement the SGP30 I2C protocol directly here:
 *
 *   - I2C address : 0x58 (fixed by hardware)
 *   - Init command: 0x2003  (iaq_init — starts on-chip baseline algorithm)
 *   - Measure cmd : 0x2008  (measure_air_quality)
 *   - Response    : 6 bytes — [eCO2_MSB, eCO2_LSB, CRC,
 *                               TVOC_MSB, TVOC_LSB, CRC]
 *   - CRC-8       : polynomial 0x31, init 0xFF, no final XOR
 *   - Timing      : must call measure_air_quality exactly every 1 s for the
 *                   on-chip baseline compensation algorithm (datasheet §3.3)
 *
 * The DTS node (compatible = "sensirion,sgp30") is kept in the overlay for
 * documentation and to give us the I2C bus reference via DT_BUS(). No Zephyr
 * sensor driver is instantiated for it.
 */

#include "sgp30.h"

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(sgp30, LOG_LEVEL_INF);

/* ── Device tree ─────────────────────────────────────────────────────────── */
/*
 * Obtain the I2C bus that the sgp30 DTS node sits on.
 * DT_BUS() walks up to the parent (i2c0) automatically.
 */
#define SGP30_NODE DT_NODELABEL(sgp30)
#define SGP30_I2C_BUS DT_BUS(SGP30_NODE)
#define SGP30_I2C_ADDR 0x58

/* ── SGP30 command bytes ─────────────────────────────────────────────────── */
static const uint8_t CMD_IAQ_INIT[] = {0x20, 0x03};
static const uint8_t CMD_MEASURE_AIR_QUAL[] = {0x20, 0x08};

/* ── CRC-8 (polynomial 0x31, init 0xFF) ─────────────────────────────────── */
static uint8_t sgp30_crc(const uint8_t *data, size_t len)
{
  uint8_t crc = 0xFF;

  for (size_t i = 0; i < len; i++)
  {
    crc ^= data[i];
    for (int b = 0; b < 8; b++)
    {
      crc = (crc & 0x80) ? (crc << 1) ^ 0x31 : (crc << 1);
    }
  }
  return crc;
}

/* ── Thread configuration ─────────────────────────────────────────────────── */
#define SGP30_THREAD_STACK_SIZE 1024
#define SGP30_THREAD_PRIORITY 5

K_THREAD_STACK_DEFINE(sgp30_stack, SGP30_THREAD_STACK_SIZE);
static struct k_thread sgp30_thread_data;

/* ── Shared state ─────────────────────────────────────────────────────────── */
static struct sgp30_data latest;
static K_MUTEX_DEFINE(data_mutex);

/* ── Reader thread ────────────────────────────────────────────────────────── */
static void sgp30_thread_fn(void *bus_ptr, void *unused1, void *unused2)
{
  ARG_UNUSED(unused1);
  ARG_UNUSED(unused2);

  const struct device *i2c = (const struct device *)bus_ptr;
  uint8_t resp[6];
  uint16_t eco2, tvoc;
  int ret;

  uint32_t warmup_ms = CONFIG_APP_SGP30_WARMUP_S * 1000U;
  int64_t start_ms = k_uptime_get();

  /* Send iaq_init — starts the on-chip baseline algorithm */
  ret = i2c_write(i2c, CMD_IAQ_INIT, sizeof(CMD_IAQ_INIT), SGP30_I2C_ADDR);
  if (ret < 0)
  {
    LOG_ERR("iaq_init failed: %d", ret);
  }
  else
  {
    LOG_INF("SGP30 iaq_init sent. Warmup: %d s",
            CONFIG_APP_SGP30_WARMUP_S);
  }
  k_sleep(K_MSEC(10)); /* datasheet: wait ≥10 ms after iaq_init */

  while (1)
  {
    /* Send measure_air_quality command */
    ret = i2c_write(i2c, CMD_MEASURE_AIR_QUAL,
                    sizeof(CMD_MEASURE_AIR_QUAL), SGP30_I2C_ADDR);
    if (ret < 0)
    {
      LOG_ERR("measure cmd write failed: %d", ret);
      goto next_cycle;
    }

    k_sleep(K_MSEC(12)); /* datasheet: wait ≥12 ms before reading */

    /* Read 6 bytes: [eCO2_H, eCO2_L, CRC, TVOC_H, TVOC_L, CRC] */
    ret = i2c_read(i2c, resp, sizeof(resp), SGP30_I2C_ADDR);
    if (ret < 0)
    {
      LOG_ERR("measure read failed: %d", ret);
      goto next_cycle;
    }

    /* Verify CRC for both words */
    if (sgp30_crc(&resp[0], 2) != resp[2])
    {
      LOG_WRN("eCO2 CRC mismatch (got 0x%02x, expect 0x%02x)",
              resp[2], sgp30_crc(&resp[0], 2));
      goto next_cycle;
    }
    if (sgp30_crc(&resp[3], 2) != resp[5])
    {
      LOG_WRN("TVOC CRC mismatch (got 0x%02x, expect 0x%02x)",
              resp[5], sgp30_crc(&resp[3], 2));
      goto next_cycle;
    }

    eco2 = ((uint16_t)resp[0] << 8) | resp[1];
    tvoc = ((uint16_t)resp[3] << 8) | resp[4];

    {
      struct sgp30_data tmp = {
          .eco2_ppm = eco2,
          .tvoc_ppb = tvoc,
          .valid = (k_uptime_get() - start_ms) >=
                   warmup_ms,
      };

      k_mutex_lock(&data_mutex, K_FOREVER);
      latest = tmp;
      k_mutex_unlock(&data_mutex);

      if (tmp.valid)
      {
        LOG_DBG("eCO2=%u ppm  TVOC=%u ppb",
                eco2, tvoc);
      }
      else
      {
        LOG_DBG("Warming up... eCO2=%u TVOC=%u",
                eco2, tvoc);
      }
    }

  next_cycle:
    /*
     * SGP30 baseline algorithm requires exactly 1 Hz sampling.
     * We already spent ~12 ms waiting for the measurement above,
     * so sleep the remainder of the 1-second period.
     */
    k_sleep(K_MSEC(CONFIG_APP_SENSOR_POLL_MS - 12));
  }
}

/* ── Public API ───────────────────────────────────────────────────────────── */

int sgp30_reader_init(void)
{
  const struct device *i2c = DEVICE_DT_GET(SGP30_I2C_BUS);

  if (!device_is_ready(i2c))
  {
    LOG_ERR("I2C bus not ready: %s", i2c->name);
    return -ENODEV;
  }

  LOG_INF("SGP30 on I2C bus %s @ 0x%02x", i2c->name, SGP30_I2C_ADDR);

  k_thread_create(&sgp30_thread_data,
                  sgp30_stack,
                  K_THREAD_STACK_SIZEOF(sgp30_stack),
                  sgp30_thread_fn,
                  (void *)i2c, NULL, NULL,
                  SGP30_THREAD_PRIORITY, 0, K_NO_WAIT);

  k_thread_name_set(&sgp30_thread_data, "sgp30");
  return 0;
}

void sgp30_get_latest(struct sgp30_data *out)
{
  k_mutex_lock(&data_mutex, K_FOREVER);
  *out = latest;
  k_mutex_unlock(&data_mutex);
}