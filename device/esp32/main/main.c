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

#define USB_CHECK_MS 500

static bool usb_power;

/* Shows the battery level when USB power is plugged in. An LVGL timer: runs under the display lock. */
static void usb_check(lv_timer_t * t)
{
    LV_UNUSED(t);
    bool usb = hal_esp32_usb_power();
    if(usb && !usb_power) brushy_ui_charging(hal_esp32_battery_percent());
    usb_power = usb;
}

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
    const uint32_t n = sizeof(bench_steps) / sizeof(bench_steps[0]);
    const char * theme, * what;
    do { /* skip the steps of themes that aren't built in */
        theme = bench_steps[i].theme;
        what = bench_steps[i].what;
        i = (i + 1) % n;
    } while(!brushy_ui_set_theme(theme));
    ESP_LOGI(TAG, "bench: %s %s", theme, what);
    if(strcmp(what, "idle") == 0 || strcmp(what, "sleepy") == 0 || strcmp(what, "brushing") == 0) {
        brushy_ui_preview(what, 20);
    }
    else brushy_ui_trick(what);
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
    bsp_display_brightness_set(CONFIG_BRUSHY_BRIGHTNESS); /* same bus as the frames, so under the lock too */
    brushy_ui_create(lv_screen_active());
    int battery = hal_esp32_battery_percent();
    usb_power = hal_esp32_usb_power();
    if(!usb_power) brushy_ui_battery(battery); /* warns when low */
    else if(hal_esp32_switched_on_by_usb()) brushy_ui_charging(battery);
    lv_timer_create(usb_check, USB_CHECK_MS, NULL);
#if CONFIG_BRUSHY_BENCH
    lv_timer_create(bench_step, 6000, NULL);
#endif
    bsp_display_unlock();

    ESP_LOGI(TAG, "running; free heap: %u KB internal, %u KB PSRAM",
             (unsigned)(heap_caps_get_free_size(MALLOC_CAP_INTERNAL) / 1024),
             (unsigned)(heap_caps_get_free_size(MALLOC_CAP_SPIRAM) / 1024));
}
