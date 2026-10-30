#pragma once

/** Open settings storage (NVS), the speaker and the power chip. Call after bsp_display_start(),
 *  which brings up the I2C bus the audio codec and the power chip sit on. */
void hal_esp32_init(void);

/** Battery charge, 0-100, or -1 when unknown: no battery, or powered over USB. */
int hal_esp32_battery_percent(void);
