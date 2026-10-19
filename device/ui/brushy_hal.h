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

/** Persisted reward counter. */
uint32_t brushy_hal_load_stars(void);
void brushy_hal_save_stars(uint32_t stars);
