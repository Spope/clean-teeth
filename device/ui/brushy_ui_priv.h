/**
 * Internal state shared by the UI modules:
 *   brushy_ui.c      phases, buddy moods, ring, overlays, input, public API
 *   brushy_fx.c      germs and their defeat, particles, shake, sparkles, reward drop
 *   brushy_tricks.c  buddy tricks (keyframed movement + timed events)
 */
#pragma once

#include "brushy_ui.h"
#include "brushy_hal.h"

#define TOTAL_MS        120000
#define ZONES           4
#define GERMS_PER_ZONE  3
#define GERM_COUNT      (ZONES * GERMS_PER_ZONE)

#define SCREEN_SIZE 466
#define CENTER      (SCREEN_SIZE / 2)
#define CHAR_SIZE   250
#define BUDDY_X     (CENTER - CHAR_SIZE / 2) /* top-left of the buddy box on screen */
#define BUDDY_Y     BUDDY_X
#define GERM_SIZE   52

typedef enum { PHASE_IDLE, PHASE_BRUSHING, PHASE_DONE } phase_t;
typedef enum { MOOD_IDLE, MOOD_SLEEPY, MOOD_BRUSHING, MOOD_CHEER } mood_t;
typedef enum { EASE_LINEAR, EASE_IN, EASE_OUT, EASE_BACK } ease_t;

typedef struct {
    lv_obj_t * img;
    int32_t cx, cy; /* centre on screen */
    uint8_t kind;   /* which of the theme's 4 germs */
    uint32_t pop_at_ms;
    bool popped;    /* claimed: defeated or being defeated */
} germ_t;

/* One keyframe: an offset from the normal place, around the object's pivot. */
typedef struct {
    uint16_t ms;
    int16_t x, y;
    uint16_t scale; /* 256 = 1x */
    int16_t rot;    /* 0.1 degree, clockwise */
    uint8_t ease;   /* ease_t of the segment that starts here */
} keyframe_t;

#define FRAME(ms, x, y, s, r, e) \
    ((keyframe_t){(uint16_t)(ms), (int16_t)(x), (int16_t)(y), (uint16_t)((s) * 256), (int16_t)((r) * 10), e})

typedef struct {
    const brushy_theme_t * theme;
    lv_obj_t * screen;
    lv_obj_t * stage; /* holds everything, so the whole picture can shake */
    lv_obj_t * character;
    lv_obj_t * arcs[ZONES];
    lv_obj_t * clock;     /* big faint countdown above the buddy */
    int32_t clock_secs;   /* seconds shown, -1 = none */
    germ_t germs[GERM_COUNT];
    lv_obj_t * stars_row;
    lv_obj_t * digits;
    lv_obj_t * hint;
    lv_obj_t * bravo;

    phase_t phase;
    mood_t mood;
    brushy_pose_t pose;
    brushy_pose_t pose_override; /* BRUSHY_POSE_COUNT = none */
    uint32_t pose_override_until; /* 0 = until cleared */
    bool flying;                  /* flap between FLY_A and FLY_B */
    float speed;
    float elapsed_ms;
    int zone;
    uint32_t phase_since;
    uint32_t mood_since;
    uint32_t last_tick;
    uint32_t stars;
    bool quiet;       /* no sounds while jumping to a preview state */
    int giggle_token; /* address used as the giggle animation's var */
} brushy_ui_t;

extern brushy_ui_t ui;

/* brushy_ui.c */
void ui_play(const brushy_note_t * notes, uint32_t count);
void ui_play_melody(const brushy_note_t * notes, uint32_t count); /* in the theme's voice */
#define SOUND(snd)  ui_play(snd, sizeof(snd) / sizeof(snd[0]))
#define MELODY(snd) ui_play_melody(snd, sizeof(snd) / sizeof(snd[0]))
lv_obj_t * ui_image(lv_obj_t * parent, const lv_image_dsc_t * src);
lv_obj_t * ui_plain_obj(lv_obj_t * parent);
void ui_anim_loop(void * var, lv_anim_exec_xcb_t cb, int32_t from, int32_t to, uint32_t ms, uint32_t delay);
void ui_exec_rotation(void * obj, int32_t v);
void ui_start_mood_motion(mood_t mood);
void ui_set_mood(mood_t mood);
void ui_override_pose(brushy_pose_t pose, uint32_t ms); /* ms = 0: until cleared */
void ui_clear_pose_override(void);
void ui_giggle(void);
void ui_render_stars(bool bump, uint32_t value);
float ui_ease(uint8_t ease, float p);
/* Interpolates frames at time t into x, y, scale, rot; steps > 0 moves in that many jumps. */
void ui_keyframes(const keyframe_t * f, uint8_t n, int32_t t, uint8_t steps, int32_t out[4]);

/* brushy_fx.c */
void fx_build_germs(void);
void fx_reset_germs(void);
void fx_pop_germ_quietly(germ_t * g);
void fx_defeat_germ(germ_t * g);
void fx_burst(int32_t x, int32_t y, const uint32_t * colors, int count, float spread);
void fx_shake(void);
void fx_mirror_check(void);
void fx_reward_drop(void);
void fx_clear(void);

/* brushy_tricks.c */
typedef enum { TRICK_PEEK, TRICK_PEEK_SIDE, TRICK_SLIDE, TRICK_MINE, TRICK_RAINBOW, TRICK_FLY, TRICK_COUNT } trick_t;
void tricks_schedule(void);
void tricks_update(uint32_t now); /* starts a random trick when it's time */
bool tricks_start(trick_t trick); /* false if the theme doesn't have it */
void tricks_stop(void);
bool tricks_active(void);
