/**
 * Brushy Buddy on the Waveshare ESP32-S3 Touch AMOLED 1.75.
 * The board package starts the display, touch and the LVGL task; the UI is device/ui/.
 */
#include "brushy_ui.h"
#include "hal_esp32.h"

#include "bsp/esp-bsp.h"
#include "esp_heap_caps.h"
#include "esp_log.h"

#include <string.h>

static const char * TAG = "brushy";

#if CONFIG_BRUSHY_BENCH
/* Plays every scene and trick in turn so LVGL's perf monitor logs the frame rate of each */
static const struct { const char * theme, * what; } bench_steps[] = {
    {"minecraft", "idle"}, {"minecraft", "sleepy"}, {"minecraft", "brushing"}, {"minecraft", "peek"},
    {"minecraft", "peek_side"}, {"minecraft", "slide"}, {"minecraft", "mine"}, {"minecraft", "xp"},
    {"minecraft", "shake"},
    {"dragon", "idle"}, {"dragon", "sleepy"}, {"dragon", "brushing"}, {"dragon", "peek"},
    {"dragon", "peek_side"}, {"dragon", "slide"}, {"dragon", "rainbow"}, {"dragon", "fly"},
    {"dragon", "shake"},
};

static void bench_step(lv_timer_t * t)
{
    static uint32_t i;
    const char * theme = bench_steps[i].theme, * what = bench_steps[i].what;
    ESP_LOGI(TAG, "bench: %s %s", theme, what);
    brushy_ui_set_theme(theme);
    if(strcmp(what, "idle") == 0 || strcmp(what, "sleepy") == 0 || strcmp(what, "brushing") == 0) {
        brushy_ui_preview(what, 20);
    }
    else brushy_ui_trick(what);
    i = (i + 1) % (sizeof(bench_steps) / sizeof(bench_steps[0]));
}
#endif

void app_main(void)
{
    if(!bsp_display_start()) {
        ESP_LOGE(TAG, "display init failed");
        return;
    }
    hal_esp32_init();

    bsp_display_lock(-1); /* LVGL runs in the adapter's task: take its lock for any lv_ call */
    brushy_ui_create(lv_screen_active());
    brushy_ui_battery(hal_esp32_battery_percent());
#if CONFIG_BRUSHY_BENCH
    lv_timer_create(bench_step, 6000, NULL);
#endif
    bsp_display_unlock();

    ESP_LOGI(TAG, "running; free heap: %u KB internal, %u KB PSRAM",
             (unsigned)(heap_caps_get_free_size(MALLOC_CAP_INTERNAL) / 1024),
             (unsigned)(heap_caps_get_free_size(MALLOC_CAP_SPIRAM) / 1024));
}
