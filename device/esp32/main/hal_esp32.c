/**
 * ESP32-S3 implementation of brushy_hal.h:
 *   sound   - brushy_synth.c, rendered in an audio task and played through the ES8311 codec
 *   storage - NVS, namespace "brushy"
 */
#include "brushy_hal.h"
#include "brushy_synth.h"
#include "hal_esp32.h"

#include <string.h>
#include "bsp/esp-bsp.h"
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

void hal_esp32_init(void)
{
    storage_init();
    audio_init();
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
