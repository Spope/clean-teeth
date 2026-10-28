/**
 * Simulator implementation of brushy_hal.h:
 *   sound   - brushy_synth.c, queued to SDL audio
 *   storage - settings in ~/.brushy-buddy-sim, one "key value" per line
 */
#include "brushy_hal.h"
#include "brushy_synth.h"
#include "hal_sim.h"

#include <SDL2/SDL.h>
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

void brushy_hal_play(const brushy_note_t * notes, uint32_t count)
{
    if(muted || !audio_dev) return;

    uint32_t total = brushy_synth_length(notes, count, SAMPLE_RATE);
    float * buf = malloc(total * sizeof(float));
    if(!buf) return;
    brushy_synth_render(notes, count, SAMPLE_RATE, buf);
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
