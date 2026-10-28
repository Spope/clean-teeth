#pragma once

/** Open settings storage (NVS) and the speaker. Call after bsp_display_start(),
 *  which brings up the I2C bus the audio codec sits on. */
void hal_esp32_init(void);
