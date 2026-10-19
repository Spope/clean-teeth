/**
 * Brushy Buddy UI: a 2-minute brushing timer for a 466x466 round screen.
 * Platform-independent; needs only LVGL and brushy_hal.h.
 */
#pragma once

#include "lvgl.h"
#include "brushy_character.h"

/** Build the UI on `screen` (expected 466x466) and start its update timer. */
void brushy_ui_create(lv_obj_t * screen, const brushy_character_t * character);

/** Time multiplier for testing (1 = real time). */
void brushy_ui_set_speed(float speed);

/** Jump to a state: "idle", "sleepy", "brushing" (at `seconds`), or "done". */
void brushy_ui_preview(const char * state, float seconds);
