#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>
#include "sgp30.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

#define SAMPLE_INTERVAL_MS 1000

int main(void)
{
  const struct device *i2c = DEVICE_DT_GET(DT_NODELABEL(i2c0));
  sgp30_t sgp30 = {0};

  if (sgp30_init(&sgp30, i2c) != 0)
  {
    LOG_ERR("SGP30 init failed — halting");
    return -1;
  }

  while (1)
  {
    uint16_t eco2, tvoc;

    if (sgp30_measure(&sgp30, &eco2, &tvoc) == 0)
    {
      LOG_INF("eCO2: %4u ppm  |  TVOC: %4u ppb", eco2, tvoc);
    }

    k_msleep(SAMPLE_INTERVAL_MS);
  }

  return 0;
}