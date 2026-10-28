/**
 * Brushy Buddy on the Waveshare ESP32-S3 Touch AMOLED 1.75.
 * The board package starts the display, touch and the LVGL task; the UI is device/ui/.
 */
#include "brushy_ui.h"
#include "hal_esp32.h"

#include "bsp/esp-bsp.h"
#include "esp_heap_caps.h"
#include "esp_log.h"

static const char * TAG = "brushy";

void app_main(void)
{
    if(!bsp_display_start()) {
        ESP_LOGE(TAG, "display init failed");
        return;
    }
    hal_esp32_init();

    bsp_display_lock(-1); /* LVGL runs in the adapter's task: take its lock for any lv_ call */
    brushy_ui_create(lv_screen_active());
    bsp_display_unlock();

    ESP_LOGI(TAG, "running; free heap: %u KB internal, %u KB PSRAM",
             (unsigned)(heap_caps_get_free_size(MALLOC_CAP_INTERNAL) / 1024),
             (unsigned)(heap_caps_get_free_size(MALLOC_CAP_SPIRAM) / 1024));
}
