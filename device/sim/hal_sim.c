/**
 * Simulator implementation of brushy_hal.h:
 *   sound   - tiny synthesizer queued to SDL audio
 *   storage - settings in ~/.brushy-buddy-sim, one "key value" per line
 */
#include "brushy_hal.h"
#include "hal_sim.h"

#include <SDL2/SDL.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SAMPLE_RATE 44100

static SDL_AudioDeviceID audio_dev;
static bool muted;

void hal_sim_init(bool mute)
{
    muted = mute;
    if(mute || SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) return;

    SDL_AudioSpec want = {0}, have;
    want.freq = SAMPLE_RATE;
    want.format = AUDIO_F32SYS;
    want.channels = 1;
    want.samples = 512;
    audio_dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if(audio_dev) SDL_PauseAudioDevice(audio_dev, 0);
    else fprintf(stderr, "audio disabled: %s\n", SDL_GetError());
}

static float wave_sample(uint8_t wave, double phase)
{
    double s = sin(phase);
    switch(wave) {
        case BRUSHY_WAVE_TRIANGLE: return (float)(2.0 / M_PI * asin(s));
        case BRUSHY_WAVE_SQUARE: return s >= 0 ? 1.0f : -1.0f;
        default: return (float)s;
    }
}

void brushy_hal_play(const brushy_note_t * notes, uint32_t count)
{
    if(muted || !audio_dev) return;

    uint32_t total = 0;
    for(uint32_t i = 0; i < count; i++) total += notes[i].ms * SAMPLE_RATE / 1000;
    float * buf = calloc(total, sizeof(float));
    if(!buf) return;

    float * out = buf;
    double phase = 0;
    float noise_lp = 0; /* low-pass state for the noise "fwoosh" */
    for(uint32_t i = 0; i < count; i++) {
        const brushy_note_t * n = &notes[i];
        uint32_t len = n->ms * SAMPLE_RATE / 1000;
        float gain = n->volume / 100.0f * 0.3f;
        for(uint32_t s = 0; s < len && n->freq; s++) {
            double t = (double)s / SAMPLE_RATE;
            double dur = n->ms / 1000.0;
            double f = n->freq_end ? n->freq * pow((double)n->freq_end / n->freq, t / dur) : n->freq;
            double env = fmin(1.0, t / 0.01) * exp(-4.0 * t / dur); /* quick attack, smooth decay */
            float v;
            if(n->wave == BRUSHY_WAVE_NOISE) { /* noise through a sweeping low-pass filter */
                float k = (float)fmin(1.0, 2 * M_PI * f / SAMPLE_RATE);
                noise_lp += k * ((float)((double)rand() / RAND_MAX * 2 - 1) - noise_lp);
                v = noise_lp * 2;
            }
            else {
                phase += 2 * M_PI * f / SAMPLE_RATE;
                v = wave_sample(n->wave, phase);
            }
            out[s] = (float)(gain * env) * v;
        }
        out += len;
    }
    SDL_QueueAudio(audio_dev, buf, total * sizeof(float));
    free(buf);
}

#define MAX_KEYS 8

typedef struct {
    char key[16];
    uint32_t value;
} setting_t;

static const char * settings_path(void)
{
    static char path[1024];
    const char * home = getenv("HOME");
    snprintf(path, sizeof(path), "%s/.brushy-buddy-sim", home ? home : ".");
    return path;
}

static int read_settings(setting_t * out)
{
    int n = 0;
    FILE * f = fopen(settings_path(), "r");
    if(!f) return 0;
    unsigned v;
    while(n < MAX_KEYS && fscanf(f, "%15s %u", out[n].key, &v) == 2) out[n++].value = v;
    fclose(f);
    return n;
}

uint32_t brushy_hal_load(const char * key, uint32_t fallback)
{
    setting_t s[MAX_KEYS];
    int n = read_settings(s);
    for(int i = 0; i < n; i++) {
        if(strcmp(s[i].key, key) == 0) return s[i].value;
    }
    return fallback;
}

void brushy_hal_save(const char * key, uint32_t value)
{
    setting_t s[MAX_KEYS];
    int n = read_settings(s), i = 0;
    while(i < n && strcmp(s[i].key, key) != 0) i++;
    if(i == n) {
        if(n == MAX_KEYS) return;
        snprintf(s[n].key, sizeof(s[n].key), "%s", key);
        n++;
    }
    s[i].value = value;

    FILE * f = fopen(settings_path(), "w");
    if(!f) return;
    for(int k = 0; k < n; k++) fprintf(f, "%s %u\n", s[k].key, (unsigned)s[k].value);
    fclose(f);
}
