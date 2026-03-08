#include "sgp30.h"

#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(sgp30, LOG_LEVEL_DBG);

#define SGP30_I2C_ADDR 0x58

/* 16-bit commands (big-endian on the wire) */
#define CMD_INIT_AIR_QUALITY 0x2003
#define CMD_MEASURE_AIR_QUALITY 0x2008

/* CRC-8 per datasheet: poly=0x31, init=0xFF */
static uint8_t crc8(const uint8_t *data, size_t len)
{
  uint8_t crc = 0xFF;

  for (size_t i = 0; i < len; i++)
  {
    crc ^= data[i];
    for (int bit = 0; bit < 8; bit++)
    {
      crc = (crc & 0x80) ? (crc << 1) ^ 0x31 : (crc << 1);
    }
  }
  return crc;
}

static int send_cmd(const struct device *i2c, uint16_t cmd)
{
  uint8_t buf[2] = {cmd >> 8, cmd & 0xFF};
  return i2c_write(i2c, buf, sizeof(buf), SGP30_I2C_ADDR);
}

int sgp30_init(sgp30_t *dev, const struct device *i2c_dev)
{
  if (!device_is_ready(i2c_dev))
  {
    LOG_ERR("I2C bus not ready");
    return -ENODEV;
  }

  dev->i2c = i2c_dev;

  int rc = send_cmd(dev->i2c, CMD_INIT_AIR_QUALITY);
  if (rc < 0)
  {
    LOG_ERR("Init_air_quality failed: %d", rc);
    return rc;
  }

  /* Datasheet: max execution time 10 ms */
  k_msleep(10);

  LOG_INF("SGP30 initialised (15 s warm-up expected)");
  return 0;
}

int sgp30_measure(sgp30_t *dev, uint16_t *eco2_ppm, uint16_t *tvoc_ppb)
{
  int rc = send_cmd(dev->i2c, CMD_MEASURE_AIR_QUALITY);
  if (rc < 0)
  {
    LOG_ERR("Measure_air_quality command failed: %d", rc);
    return rc;
  }

  /* Datasheet: max execution time 12 ms */
  k_msleep(12);

  /* Response: [CO2_H, CO2_L, CRC, TVOC_H, TVOC_L, CRC] */
  uint8_t buf[6];
  rc = i2c_read(dev->i2c, buf, sizeof(buf), SGP30_I2C_ADDR);
  if (rc < 0)
  {
    LOG_ERR("I2C read failed: %d", rc);
    return rc;
  }

  if (crc8(&buf[0], 2) != buf[2])
  {
    LOG_ERR("CRC mismatch on eCO2 word (got 0x%02x, expected 0x%02x)",
            buf[2], crc8(&buf[0], 2));
    return -EIO;
  }

  if (crc8(&buf[3], 2) != buf[5])
  {
    LOG_ERR("CRC mismatch on TVOC word (got 0x%02x, expected 0x%02x)",
            buf[5], crc8(&buf[3], 2));
    return -EIO;
  }

  *eco2_ppm = ((uint16_t)buf[0] << 8) | buf[1];
  *tvoc_ppb = ((uint16_t)buf[3] << 8) | buf[4];

  return 0;
}