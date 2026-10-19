/**
 * Simulator implementation of brushy_hal.h:
 *   sound   - tiny synthesizer queued to SDL audio
 *   storage - star count in ~/.brushy-buddy-sim
 */
#include "brushy_hal.h"
#include "hal_sim.h"

#include <SDL2/SDL.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

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
    for(uint32_t i = 0; i < count; i++) {
        const brushy_note_t * n = &notes[i];
        uint32_t len = n->ms * SAMPLE_RATE / 1000;
        float gain = n->volume / 100.0f * 0.3f;
        for(uint32_t s = 0; s < len && n->freq; s++) {
            double t = (double)s / SAMPLE_RATE;
            double dur = n->ms / 1000.0;
            double f = n->freq_end ? n->freq * pow((double)n->freq_end / n->freq, t / dur) : n->freq;
            double env = fmin(1.0, t / 0.01) * exp(-4.0 * t / dur); /* quick attack, smooth decay */
            phase += 2 * M_PI * f / SAMPLE_RATE;
            out[s] = (float)(gain * env) * wave_sample(n->wave, phase);
        }
        out += len;
    }
    SDL_QueueAudio(audio_dev, buf, total * sizeof(float));
    free(buf);
}

static const char * stars_path(void)
{
    static char path[1024];
    const char * home = getenv("HOME");
    snprintf(path, sizeof(path), "%s/.brushy-buddy-sim", home ? home : ".");
    return path;
}

uint32_t brushy_hal_load_stars(void)
{
    unsigned stars = 0;
    FILE * f = fopen(stars_path(), "r");
    if(f) {
        if(fscanf(f, "%u", &stars) != 1) stars = 0;
        fclose(f);
    }
    return stars;
}

void brushy_hal_save_stars(uint32_t stars)
{
    FILE * f = fopen(stars_path(), "w");
    if(!f) return;
    fprintf(f, "%u\n", (unsigned)stars);
    fclose(f);
}
