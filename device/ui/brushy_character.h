/**
 * A buddy character: theme colours plus one still image per pose.
 * Pose images come from the prototype's SVG (prototype/characters/<id>.js `poses`),
 * exported by tools/export_assets.sh and converted at build time.
 */
#pragma once

#include "lvgl.h"

typedef enum {
    BRUSHY_POSE_IDLE,
    BRUSHY_POSE_BLINK,
    BRUSHY_POSE_SLEEPY,
    BRUSHY_POSE_BRUSH_A,
    BRUSHY_POSE_BRUSH_B,
    BRUSHY_POSE_CHEER,
    BRUSHY_POSE_COUNT,
} brushy_pose_t;

typedef struct {
    const char * id;
    const char * name;
    uint32_t accent;  /**< 0xRRGGBB, buttons and text */
    uint32_t ring[4]; /**< 0xRRGGBB, one per brushing zone */
    const lv_image_dsc_t * poses[BRUSHY_POSE_COUNT];
} brushy_character_t;

extern const brushy_character_t brushy_dino;
