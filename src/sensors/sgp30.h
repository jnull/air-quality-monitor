#ifndef APP_SGP30_H
#define APP_SGP30_H

#include <stdint.h>
#include <stdbool.h>
#include <zephyr/kernel.h>

#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * @brief Latest air quality reading from the SGP30.
   *
   * Both values are set to 0 while the sensor is still warming up.
   */
  struct sgp30_data
  {
    uint16_t eco2_ppm; /**< Equivalent CO2 concentration, ppm  (400–8192) */
    uint16_t tvoc_ppb; /**< Total VOC concentration, ppb       (0–60000)   */
    bool valid;        /**< true once the 15-second warmup is complete     */
  };

  /**
   * @brief Initialise the SGP30 and spawn its reader thread.
   *
   * Must be called once from main() before @ref sgp30_get_latest().
   *
   * @return 0 on success, negative errno on failure.
   */
  int sgp30_reader_init(void);

  /**
   * @brief Copy the most recent reading into @p out.
   *
   * Thread-safe; may be called from any thread or ISR.
   *
   * @param out  Destination; filled with the latest valid (or warmup) data.
   */
  void sgp30_get_latest(struct sgp30_data *out);

#ifdef __cplusplus
}
#endif
#endif /* APP_SGP30_H */