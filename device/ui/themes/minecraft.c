#include "brushy_theme.h"
#include "brushy_hal.h"
#include "assets.h"

/* Mirrors prototype/themes/minecraft.js. */
const brushy_theme_t brushy_theme_minecraft = {
    .id = "minecraft",
    .name = "Minecraft - Steve",
    .style = BRUSHY_STYLE_BLOCKY,
    .accent = 0x7ac943,
    .ring = {0x7ac943, 0xf9d71c, 0x5ce1e6, 0xe23c3c}, /* grass, gold, diamond, redstone */
    .voice = BRUSHY_WAVE_SQUARE,
    .pop_whoosh = false,
    .pop = BRUSHY_POP_CREEPER,
    .tricks = BRUSHY_TRICK_MINE,
    .teeth_x = 125, .teeth_y = 114,
    .mouth_x = 125, .mouth_y = 114,
    .germ_particles = {
        {0x5dbb3a, 0x79d24f, 0x4a9c2c, 0x10200c}, /* creeper */
        {0x5fa896, 0x4a8a7a, 0x2c4a3a},           /* zombie */
        {0x6cc45c, 0x93e283, 0xb5f0a8},           /* slime */
        {0x3b3535, 0x2a2525, 0xe0383e},           /* spider */
    },
    BRUSHY_THEME_IMAGES(minecraft),
    .poses[BRUSHY_POSE_MINE_A] = &img_minecraft_mine_a,
    .poses[BRUSHY_POSE_MINE_B] = &img_minecraft_mine_b,
    .poses[BRUSHY_POSE_MINE_BRUSH_A] = &img_minecraft_mine_brush_a,
    .poses[BRUSHY_POSE_MINE_BRUSH_B] = &img_minecraft_mine_brush_b,
    .block = &img_minecraft_block,
    .block_cracked = &img_minecraft_block_cracked,
};
