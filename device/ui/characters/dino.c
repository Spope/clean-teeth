#include "brushy_character.h"
#include "assets.h"

/* Colours mirror `theme` in prototype/characters/dino.js. */
const brushy_character_t brushy_dino = {
    .id = "dino",
    .name = "Dino",
    .accent = 0x5bd48a,
    .ring = {0x7ce38b, 0xffd166, 0xff8fab, 0x7cc8ff},
    .poses = {
        [BRUSHY_POSE_IDLE] = &img_dino_idle,
        [BRUSHY_POSE_BLINK] = &img_dino_blink,
        [BRUSHY_POSE_SLEEPY] = &img_dino_sleepy,
        [BRUSHY_POSE_BRUSH_A] = &img_dino_brush_a,
        [BRUSHY_POSE_BRUSH_B] = &img_dino_brush_b,
        [BRUSHY_POSE_CHEER] = &img_dino_cheer,
    },
};
