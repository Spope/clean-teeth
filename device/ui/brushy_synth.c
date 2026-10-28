/**
 * Synthesizer for brushy_hal_play(): sine / triangle / square tones with an
 * exponential pitch slide, and filtered noise for the "fwoosh".
 * Single-precision maths only (the ESP32-S3 has no double-precision FPU).
 */
#include "brushy_synth.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#define PI_F 3.14159265f

static float wave_sample(uint8_t wave, float phase)
{
    float s = sinf(phase);
    switch(wave) {
        case BRUSHY_WAVE_TRIANGLE: return 2.0f / PI_F * asinf(s);
        case BRUSHY_WAVE_SQUARE: return s >= 0 ? 1.0f : -1.0f;
        default: return s;
    }
}

uint32_t brushy_synth_length(const brushy_note_t * notes, uint32_t count, uint32_t rate)
{
    uint32_t total = 0;
    for(uint32_t i = 0; i < count; i++) total += notes[i].ms * rate / 1000;
    return total;
}

void brushy_synth_render(const brushy_note_t * notes, uint32_t count, uint32_t rate, float * out)
{
    float phase = 0;
    float noise_lp = 0; /* low-pass state for the noise "fwoosh" */
    for(uint32_t i = 0; i < count; i++) {
        const brushy_note_t * n = &notes[i];
        uint32_t len = n->ms * rate / 1000;
        if(!n->freq) { /* rest */
            memset(out, 0, len * sizeof(float));
            out += len;
            continue;
        }
        float gain = n->volume / 100.0f * 0.3f;
        float dur = n->ms / 1000.0f;
        for(uint32_t s = 0; s < len; s++) {
            float t = (float)s / rate;
            float f = n->freq_end ? n->freq * powf((float)n->freq_end / n->freq, t / dur) : n->freq;
            float env = fminf(1.0f, t / 0.01f) * expf(-4.0f * t / dur); /* quick attack, smooth decay */
            float v;
            if(n->wave == BRUSHY_WAVE_NOISE) { /* noise through a sweeping low-pass filter */
                float k = fminf(1.0f, 2 * PI_F * f / rate);
                noise_lp += k * ((float)rand() / (float)RAND_MAX * 2 - 1 - noise_lp);
                v = noise_lp * 2;
            }
            else {
                phase += 2 * PI_F * f / rate;
                if(phase > 2 * PI_F) phase -= 2 * PI_F; /* keep float precision */
                v = wave_sample(n->wave, phase);
            }
            out[s] = gain * env * v;
        }
        out += len;
    }
}
