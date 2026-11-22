/**
 * Brushy Buddy UI: a 2-minute brushing timer for a 466x466 round screen.
 * Platform-independent; needs only LVGL and brushy_hal.h.
 */
#pragma once

#include "lvgl.h"
#include "brushy_theme.h"

/** Build the UI on `screen` (expected 466x466) and start its update timer.
 *  Starts with the theme saved last time, or the first one. */
void brushy_ui_create(lv_obj_t * screen);

/** Battery charge (%) at or below which brushy_ui_battery() warns. */
#define BRUSHY_LOW_BATTERY 25

/** Battery charge at boot, 0-100 or -1 if unknown: at BRUSHY_LOW_BATTERY or below, shows a
 *  low battery warning for a few seconds (a tap dismisses it). */
void brushy_ui_battery(int percent);

/** Battery charge when USB power is plugged in, 0-100 or -1 if unknown: shows the level with a
 *  charging bolt for a few seconds (a tap dismisses it). Wakes the buddy if asleep and restarts
 *  the power-off countdown. */
void brushy_ui_charging(int percent);

/** Switch theme by id (e.g. "minecraft", "dragon"). Returns false if unknown. */
bool brushy_ui_set_theme(const char * id);

/** The theme shown now. */
const brushy_theme_t * brushy_ui_theme(void);

/** Play an animation now: "peek" (peek-a-boo), "peek_side", "slide", "mine", "xp",
 *  "rainbow", "fly", "mirror" or "shake". Returns false if unknown or not in the current theme. */
bool brushy_ui_trick(const char * name);

/** Whether brushy_ui_trick(name) can play it in the current theme. */
bool brushy_ui_has_trick(const char * name);

/** Time multiplier for testing (1 = real time). */
void brushy_ui_set_speed(float speed);

/** Jump to a state: "idle", "sleepy", "brushing" (at `seconds`), or "done". */
void brushy_ui_preview(const char * state, float seconds);

/** The state shown now, as brushy_ui_preview() names them, and the brushing time so far
 *  (seconds, up to BRUSHY_BRUSHING_SECONDS): for a status display. */
const char * brushy_ui_state(void);
float brushy_ui_elapsed(void);

/** How long brushing lasts. */
#define BRUSHY_BRUSHING_SECONDS 120
