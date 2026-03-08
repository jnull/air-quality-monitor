#ifndef SGP30_H
#define SGP30_H

#include <zephyr/device.h>
#include <stdint.h>

/** Opaque driver handle — zero-initialise before calling sgp30_init(). */
typedef struct
{
  const struct device *i2c;
} sgp30_t;

/**
 * Initialise the SGP30 on the given I2C bus.
 * Sends Init_air_quality and blocks for the required 10 ms.
 * Returns 0 on success, negative errno on failure.
 */
int sgp30_init(sgp30_t *dev, const struct device *i2c_dev);

/**
 * Read eCO2 (ppm) and TVOC (ppb).
 * The caller must invoke this every ~1 s for the on-chip baseline algorithm.
 * For the first 15 s after init the values will be 400 ppm / 0 ppb — that is
 * normal sensor behaviour, not an error.
 * Returns 0 on success, negative errno on failure (including CRC mismatch).
 */
int sgp30_measure(sgp30_t *dev, uint16_t *eco2_ppm, uint16_t *tvoc_ppb);

#endif /* SGP30_H */