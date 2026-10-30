/**
 * ESP32-S3 implementation of brushy_hal.h:
 *   sound   - brushy_synth.c, rendered in an audio task and played through the ES8311 codec
 *   storage - NVS, namespace "brushy"
 *   battery - the AXP2101 power chip's fuel gauge, on the BSP's I2C bus
 */
#include "brushy_hal.h"
#include "brushy_synth.h"
#include "hal_esp32.h"

#include <string.h>
#include "bsp/esp-bsp.h"
#include "driver/i2c_master.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"

#define SAMPLE_RATE 22050 /* the BSP's default I2S setup */
#define VOLUME 70         /* codec output volume, 0-100 */
#define MAX_NOTES 16      /* longest sequence the UI plays */
#define QUEUE_LEN 4
#define NVS_NAMESPACE "brushy"

#define AXP2101_ADDR        0x34
#define AXP2101_STATUS1     0x00 /* bit 5: USB power good, bit 3: battery connected */
#define AXP2101_STATUS2     0x01 /* bits 7-5: 001 = charging */
#define AXP2101_BAT_DET     0x68 /* bit 0: battery detection on */
#define AXP2101_BAT_PERCENT 0xA4 /* fuel gauge, 0-100 */

static const char * TAG = "hal";

typedef struct {
    uint32_t count;
    brushy_note_t notes[MAX_NOTES]; /* copied: the UI may pass a stack buffer */
} sound_t;

static QueueHandle_t sounds;
static esp_codec_dev_handle_t speaker;

/* Plays queued sounds one after the other, like SDL_QueueAudio in the simulator. */
static void audio_task(void * arg)
{
    (void)arg;
    sound_t s;
    while(1) {
        xQueueReceive(sounds, &s, portMAX_DELAY);
        uint32_t n = brushy_synth_length(s.notes, s.count, SAMPLE_RATE);
        float * wave = heap_caps_malloc(n * sizeof(float), MALLOC_CAP_SPIRAM);
        int16_t * pcm = heap_caps_malloc(n * sizeof(int16_t), MALLOC_CAP_SPIRAM);
        if(n && wave && pcm) {
            brushy_synth_render(s.notes, s.count, SAMPLE_RATE, wave);
            for(uint32_t i = 0; i < n; i++) {
                float v = wave[i] < -1 ? -1 : wave[i] > 1 ? 1 : wave[i];
                pcm[i] = (int16_t)(v * 32767);
            }
            esp_codec_dev_write(speaker, pcm, (int)(n * sizeof(int16_t))); /* blocks while playing */
        }
        heap_caps_free(wave);
        heap_caps_free(pcm);
    }
}

static void audio_init(void)
{
    speaker = bsp_audio_codec_speaker_init();
    if(!speaker) {
        ESP_LOGE(TAG, "speaker codec init failed, sound disabled");
        return;
    }
    esp_codec_dev_sample_info_t fs = {
        .bits_per_sample = 16,
        .channel = 1,
        .sample_rate = SAMPLE_RATE,
    };
    if(esp_codec_dev_open(speaker, &fs) != ESP_CODEC_DEV_OK) {
        ESP_LOGE(TAG, "speaker open failed, sound disabled");
        speaker = NULL;
        return;
    }
    esp_codec_dev_set_out_vol(speaker, VOLUME);

    sounds = xQueueCreate(QUEUE_LEN, sizeof(sound_t));
    xTaskCreate(audio_task, "audio", 4096, NULL, 5, NULL);
}

static void storage_init(void)
{
    esp_err_t err = nvs_flash_init();
    if(err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}

static i2c_master_dev_handle_t pmu;

static bool pmu_read(uint8_t reg, uint8_t * value)
{
    return i2c_master_transmit_receive(pmu, &reg, 1, value, 1, 100) == ESP_OK;
}

static void battery_init(void)
{
    i2c_device_config_t cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = AXP2101_ADDR,
        .scl_speed_hz = 400000,
    };
    uint8_t det;
    if(i2c_master_bus_add_device(bsp_i2c_get_handle(), &cfg, &pmu) != ESP_OK || !pmu_read(AXP2101_BAT_DET, &det)) {
        ESP_LOGW(TAG, "AXP2101 not found, battery level unknown");
        pmu = NULL;
        return;
    }
    uint8_t on[2] = {AXP2101_BAT_DET, det | 1};
    i2c_master_transmit(pmu, on, sizeof(on), 100);
}

int hal_esp32_battery_percent(void)
{
    uint8_t status1, status2, percent;
    if(!pmu || !pmu_read(AXP2101_STATUS1, &status1) || !pmu_read(AXP2101_STATUS2, &status2)) return -1;
    if(!(status1 & (1 << 3))) return -1; /* no battery */
    bool usb = status1 & (1 << 5), charging = (status2 >> 5) == 1;
    if(!pmu_read(AXP2101_BAT_PERCENT, &percent) || percent > 100) return -1;
    ESP_LOGI(TAG, "battery %u%%%s", percent, charging ? ", charging" : usb ? ", on USB" : "");
    return usb ? -1 : percent; /* plugged in: no need to warn */
}

void hal_esp32_init(void)
{
    storage_init();
    audio_init();
    battery_init();
}

void brushy_hal_play(const brushy_note_t * notes, uint32_t count)
{
    if(!sounds || !count) return;
    sound_t s = {.count = count < MAX_NOTES ? count : MAX_NOTES};
    memcpy(s.notes, notes, s.count * sizeof(brushy_note_t));
    if(xQueueSend(sounds, &s, 0) != pdTRUE) ESP_LOGW(TAG, "sound dropped (queue full)");
}

uint32_t brushy_hal_load(const char * key, uint32_t fallback)
{
    nvs_handle_t h;
    uint32_t value = fallback;
    if(nvs_open(NVS_NAMESPACE, NVS_READONLY, &h) != ESP_OK) return fallback; /* nothing saved yet */
    if(nvs_get_u32(h, key, &value) != ESP_OK) value = fallback;
    nvs_close(h);
    return value;
}

void brushy_hal_save(const char * key, uint32_t value)
{
    nvs_handle_t h;
    if(nvs_open(NVS_NAMESPACE, NVS_READWRITE, &h) != ESP_OK) return;
    if(nvs_set_u32(h, key, value) == ESP_OK) nvs_commit(h);
    nvs_close(h);
}
