#pragma once

#include <stdbool.h>

/** Open settings storage (NVS), the speaker and the power chip. Call after bsp_display_start(),
 *  which brings up the I2C bus the audio codec and the power chip sit on. */
void hal_esp32_init(void);

/** Battery charge, 0-100, or -1 when unknown (no battery). */
int hal_esp32_battery_percent(void);

/** USB power is plugged in. */
bool hal_esp32_usb_power(void);

/** The power chip switched the device on because USB power was plugged in (rather than the PWR
 *  button). It keeps saying so after a reset of the ESP32 alone, e.g. after flashing. */
bool hal_esp32_switched_on_by_usb(void);
