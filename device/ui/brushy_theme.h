/**
 * A theme: buddy poses, germs, sprites, lettering, colours and sound flavour.
 * All images come from the prototype (prototype/themes/<id>.js), exported by
 * tools/export_assets.sh and converted at build time.
 */
#pragma once

#include "lvgl.h"

typedef enum {
    /* every theme */
    BRUSHY_POSE_IDLE,
    BRUSHY_POSE_BLINK,
    BRUSHY_POSE_SLEEPY,
    BRUSHY_POSE_BRUSH_A,
    BRUSHY_POSE_BRUSH_B,
    BRUSHY_POSE_CHEER,
    BRUSHY_POSE_GRIN,      /* mirror check */
    /* only for themes with the matching trick or pop style */
    BRUSHY_POSE_MINE_A,    /* pickaxe raised */
    BRUSHY_POSE_MINE_B,    /* pickaxe hits */
    BRUSHY_POSE_FIRE,      /* breathing a fireball */
    BRUSHY_POSE_FLY_A,     /* wings up */
    BRUSHY_POSE_FLY_B,     /* wings down */
    BRUSHY_POSE_RAINBOW_1, /* ...to RAINBOW_5: scale colours */
    BRUSHY_POSE_RAINBOW_5 = BRUSHY_POSE_RAINBOW_1 + 4,
    BRUSHY_POSE_COUNT,
} brushy_pose_t;

typedef enum {
    BRUSHY_STYLE_ROUND,  /**< rounded ring ends, round button, rounded confetti */
    BRUSHY_STYLE_BLOCKY, /**< ring cut into blocks, square button and confetti, stepped motion */
} brushy_style_t;

typedef enum {
    BRUSHY_POP_BURST,    /**< germs burst into particles */
    BRUSHY_POP_CREEPER,  /**< like BURST, but germ 0 swells and flashes white first */
    BRUSHY_POP_FIREBALL, /**< the buddy breathes a fireball at each germ */
} brushy_pop_t;

/** Theme-only tricks (start screen only) */
#define BRUSHY_TRICK_MINE    (1u << 0)
#define BRUSHY_TRICK_RAINBOW (1u << 1)
#define BRUSHY_TRICK_FLY     (1u << 2)

typedef struct {
    const char * id;
    const char * name;
    brushy_style_t style;
    uint32_t accent;  /**< 0xRRGGBB, start button */
    uint32_t ring[4]; /**< 0xRRGGBB, one per brushing zone */
    uint8_t voice;    /**< brushy_wave_t used for melodies */
    bool pop_whoosh;  /**< germs pop with a "fwoosh" instead of a blip */
    brushy_pop_t pop;
    uint8_t tricks;   /**< BRUSHY_TRICK_* */
    int16_t teeth_x, teeth_y; /**< mirror-check sparkle, in the 250x250 buddy box */
    int16_t mouth_x, mouth_y; /**< where fireballs start, same box */
    uint32_t germ_particles[4][4]; /**< 0xRRGGBB burst colours per germ, 0 = unused */

    const lv_image_dsc_t * poses[BRUSHY_POSE_COUNT]; /**< NULL when the theme doesn't need it */
    const lv_image_dsc_t * germs[4];
    const lv_image_dsc_t * germs_scared[4];
    const lv_image_dsc_t * reward;
    const lv_image_dsc_t * bravo;
    const lv_image_dsc_t * digits[10];
    const lv_image_dsc_t * sparkle;
    const lv_image_dsc_t * block;         /**< mine trick */
    const lv_image_dsc_t * block_cracked; /**< mine trick */
    const lv_image_dsc_t * fireball;      /**< BRUSHY_POP_FIREBALL */
} brushy_theme_t;

/** Fills in the images every theme has, from the asset naming scheme (<id>_idle, ...). */
#define BRUSHY_THEME_IMAGES(t) \
    .poses[BRUSHY_POSE_IDLE] = &img_##t##_idle, \
    .poses[BRUSHY_POSE_BLINK] = &img_##t##_blink, \
    .poses[BRUSHY_POSE_SLEEPY] = &img_##t##_sleepy, \
    .poses[BRUSHY_POSE_BRUSH_A] = &img_##t##_brush_a, \
    .poses[BRUSHY_POSE_BRUSH_B] = &img_##t##_brush_b, \
    .poses[BRUSHY_POSE_CHEER] = &img_##t##_cheer, \
    .poses[BRUSHY_POSE_GRIN] = &img_##t##_grin, \
    .germs = {&img_##t##_germ_0, &img_##t##_germ_1, &img_##t##_germ_2, &img_##t##_germ_3}, \
    .germs_scared = {&img_##t##_germ_scared_0, &img_##t##_germ_scared_1, \
                     &img_##t##_germ_scared_2, &img_##t##_germ_scared_3}, \
    .reward = &img_##t##_reward, \
    .bravo = &img_##t##_bravo, \
    .digits = {&img_##t##_digit_0, &img_##t##_digit_1, &img_##t##_digit_2, &img_##t##_digit_3, \
               &img_##t##_digit_4, &img_##t##_digit_5, &img_##t##_digit_6, &img_##t##_digit_7, \
               &img_##t##_digit_8, &img_##t##_digit_9}, \
    .sparkle = &img_##t##_sparkle

extern const brushy_theme_t * const brushy_themes[];
extern const uint32_t brushy_theme_count;
