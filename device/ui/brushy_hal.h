/**
 * Platform services the UI needs. Implemented once per target:
 *   device/sim/hal_sim.c  - desktop simulator (SDL audio, file storage)
 *   (later) ESP32-S3      - audio codec + NVS
 */
#pragma once

#include <stdint.h>

typedef enum {
    BRUSHY_WAVE_SINE,
    BRUSHY_WAVE_TRIANGLE,
    BRUSHY_WAVE_SQUARE,
    BRUSHY_WAVE_NOISE, /**< filtered noise, freq -> freq_end is the filter sweep */
} brushy_wave_t;

typedef struct {
    uint16_t freq;     /**< Hz, 0 = rest */
    uint16_t freq_end; /**< Hz to slide to, 0 = constant pitch */
    uint16_t ms;       /**< duration */
    uint8_t wave;      /**< brushy_wave_t */
    uint8_t volume;    /**< 0-100 */
} brushy_note_t;

/** Play a short sequence of notes without blocking. */
void brushy_hal_play(const brushy_note_t * notes, uint32_t count);

/** Small persistent settings ("stars", "theme"); keys are at most 15 characters. */
uint32_t brushy_hal_load(const char * key, uint32_t fallback);
void brushy_hal_save(const char * key, uint32_t value);
