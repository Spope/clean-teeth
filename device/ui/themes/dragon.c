#include "brushy_theme.h"
#include "brushy_hal.h"
#include "assets.h"

/* Mirrors prototype/themes/dragon.js ("Les Royaumes de feu": Glory the RainWing). */
const brushy_theme_t brushy_theme_dragon = {
    .id = "dragon",
    .name = "Royaumes de feu - Glory",
    .style = BRUSHY_STYLE_ROUND,
    .accent = 0x3fbfa0,
    .ring = {0x3fbfa0, 0xffd23f, 0xff7ac2, 0x6fa8ff},
    .voice = BRUSHY_WAVE_TRIANGLE,
    .pop_whoosh = true,
    .pop = BRUSHY_POP_VENOM,
    .tricks = BRUSHY_TRICK_RAINBOW | BRUSHY_TRICK_FLY,
    .teeth_x = 125, .teeth_y = 134,
    .mouth_x = 125, .mouth_y = 137,
    .germ_particles = { /* jungle bugs melted by venom */
        {0x8a5cff, 0x1b1426, 0x5dd94a},
        {0xff5fa2, 0x1b1426, 0x5dd94a},
        {0x9be15d, 0x1b1426, 0x5dd94a},
        {0xff9f43, 0x1b1426, 0x5dd94a},
    },
    BRUSHY_THEME_IMAGES(dragon),
    .poses[BRUSHY_POSE_SPIT] = &img_dragon_spit,
    .poses[BRUSHY_POSE_FLY_A] = &img_dragon_fly_a,
    .poses[BRUSHY_POSE_FLY_B] = &img_dragon_fly_b,
    .poses[BRUSHY_POSE_RAINBOW_1 + 0] = &img_dragon_rainbow_1,
    .poses[BRUSHY_POSE_RAINBOW_1 + 1] = &img_dragon_rainbow_2,
    .poses[BRUSHY_POSE_RAINBOW_1 + 2] = &img_dragon_rainbow_3,
    .poses[BRUSHY_POSE_RAINBOW_1 + 3] = &img_dragon_rainbow_4,
    .poses[BRUSHY_POSE_RAINBOW_1 + 4] = &img_dragon_rainbow_5,
    .venom = &img_dragon_venom,
};
