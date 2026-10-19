/**
 * Brushy Buddy UI. Mirrors the behaviour of the browser prototype (prototype/js/app.js):
 *   idle     - buddy waits (falls asleep after 30 s); tap to start,
 *              press and hold to switch theme
 *   brushing - 2 minutes, 4 zones of 30 s filling the ring clockwise from top-left;
 *              germs are defeated over time (see brushy_fx.c), a mirror check just past
 *              halfway, tap = giggle
 *   done     - cheer, confetti, "Bravo !", the reward drops into the counter;
 *              back to idle on tap or after 12 s
 * While awake, the buddy plays a random trick every 8-14 s (see brushy_tricks.c).
 */
#include "brushy_ui_priv.h"

#include <math.h>
#include <string.h>

#define SLEEP_AFTER_MS    30000
#define CELEBRATE_MS      12000
#define DONE_TAP_GUARD_MS 1500
#define LONG_PRESS_MS     900

#define RING_DIAM       448 /* 18 px stroke centred on r = 215 */
#define RING_WIDTH      18
#define BLOCKS_PER_ZONE 8   /* blocky style */
#define CONFETTI        50

#define CLOCK_Y       88 /* centre of the countdown: the free band above the buddy's head */
#define CLOCK_OPA     82 /* about 32 % */

#define LOOP_MS       33
#define BRUSH_SWAP_MS 280
#define FLAP_MS       150
#define BLINK_EVERY   4000
#define BLINK_MS      150

brushy_ui_t ui;

/* ---------- Sounds (same melodies as prototype/js/sound.js) ---------- */

#define NOTE(f, ms) {f, 0, ms, BRUSHY_WAVE_TRIANGLE, 60}

static const brushy_note_t SND_START[] = {NOTE(523, 100), NOTE(659, 100), NOTE(784, 200)};
static const brushy_note_t SND_ZONE[] = {NOTE(784, 120), NOTE(1047, 220)};
static const brushy_note_t SND_FINISH[] = {
    NOTE(523, 120), NOTE(659, 120), NOTE(784, 120), NOTE(1047, 240),
    NOTE(0, 60), NOTE(784, 120), NOTE(1047, 400),
};
static const brushy_note_t SND_GIGGLE[] = {
    {880, 0, 60, BRUSHY_WAVE_SQUARE, 20}, {988, 0, 60, BRUSHY_WAVE_SQUARE, 20},
    {880, 0, 60, BRUSHY_WAVE_SQUARE, 20}, {1175, 0, 100, BRUSHY_WAVE_SQUARE, 20},
};

void ui_play(const brushy_note_t * notes, uint32_t count)
{
    if(!ui.quiet) brushy_hal_play(notes, count);
}

void ui_play_melody(const brushy_note_t * notes, uint32_t count)
{
    if(ui.quiet) return;
    brushy_note_t voiced[16];
    if(count > 16) count = 16;
    for(uint32_t i = 0; i < count; i++) {
        voiced[i] = notes[i];
        voiced[i].wave = ui.theme->voice;
        if(ui.theme->voice == BRUSHY_WAVE_SQUARE) voiced[i].volume /= 2; /* squares are loud */
    }
    brushy_hal_play(voiced, count);
}

/* ---------- Helpers ---------- */

static void exec_translate_y(void * obj, int32_t v) { lv_obj_set_style_translate_y(obj, v, 0); }
static void exec_arc_opa(void * obj, int32_t v) { lv_obj_set_style_arc_opa(obj, (lv_opa_t)v, LV_PART_MAIN); }
static void exec_scale(void * obj, int32_t v) { lv_obj_set_style_transform_scale(obj, v, 0); }
static void exec_y(void * obj, int32_t v) { lv_obj_set_y(obj, v); }
void ui_exec_rotation(void * obj, int32_t v) { lv_image_set_rotation(obj, v); }

/* v: 0..1000 -> slightly wider and shorter, like breathing */
static void exec_breathe(void * obj, int32_t v)
{
    lv_image_set_scale_x(obj, 256 + 8 * v / 1000);
    lv_image_set_scale_y(obj, 256 - 5 * v / 1000);
}

/* Squash-and-stretch keyframes, same as the prototype's `giggle` CSS animation */
static void exec_giggle(void * var, int32_t v)
{
    LV_UNUSED(var);
    static const float k[][4] = { /* t, scale x, scale y, rotation deg */
        {0, 1, 1, 0}, {250, 1.12f, 0.88f, 0}, {500, 0.92f, 1.1f, -4}, {750, 1.04f, 0.97f, 3}, {1000, 1, 1, 0},
    };
    int i = 0;
    while(i < 3 && v > k[i + 1][0]) i++;
    float f = (v - k[i][0]) / (k[i + 1][0] - k[i][0]);
    float sx = k[i][1] + (k[i + 1][1] - k[i][1]) * f;
    float sy = k[i][2] + (k[i + 1][2] - k[i][2]) * f;
    float rot = k[i][3] + (k[i + 1][3] - k[i][3]) * f;
    lv_image_set_scale_x(ui.character, (uint32_t)(256 * sx));
    lv_image_set_scale_y(ui.character, (uint32_t)(256 * sy));
    lv_image_set_rotation(ui.character, (int32_t)(rot * 10));
}

void ui_anim_loop(void * var, lv_anim_exec_xcb_t cb, int32_t from, int32_t to, uint32_t ms, uint32_t delay)
{
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, var);
    lv_anim_set_exec_cb(&a, cb);
    lv_anim_set_values(&a, from, to);
    lv_anim_set_duration(&a, ms);
    lv_anim_set_reverse_duration(&a, ms);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_delay(&a, delay);
    lv_anim_set_path_cb(&a, ui.theme->style == BRUSHY_STYLE_BLOCKY ? lv_anim_path_step : lv_anim_path_ease_in_out);
    lv_anim_start(&a);
}

lv_obj_t * ui_plain_obj(lv_obj_t * parent)
{
    lv_obj_t * o = lv_obj_create(parent);
    lv_obj_remove_style_all(o);
    lv_obj_remove_flag(o, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    return o;
}

lv_obj_t * ui_image(lv_obj_t * parent, const lv_image_dsc_t * src)
{
    lv_obj_t * img = lv_image_create(parent);
    lv_obj_remove_flag(img, LV_OBJ_FLAG_CLICKABLE); /* taps go to the stage */
    lv_image_set_src(img, src);
    return img;
}

float ui_ease(uint8_t e, float p)
{
    switch(e) {
        case EASE_IN: return p * p;
        case EASE_OUT: return 1 - (1 - p) * (1 - p);
        case EASE_BACK: { /* overshoots a little, then settles */
            const float c1 = 1.70158f, c3 = c1 + 1, q = p - 1;
            return 1 + c3 * q * q * q + c1 * q * q;
        }
        default: return p;
    }
}

void ui_keyframes(const keyframe_t * f, uint8_t n, int32_t t, uint8_t steps, int32_t out[4])
{
    int i = 0;
    while(i < n - 2 && t >= f[i + 1].ms) i++;
    float p = (float)(t - f[i].ms) / (float)(f[i + 1].ms - f[i].ms);
    p = p < 0 ? 0 : (p > 1 ? 1 : p);
    p = steps ? floorf(p * steps) / steps : ui_ease(f[i].ease, p);
    out[0] = (int32_t)(f[i].x + (f[i + 1].x - f[i].x) * p);
    out[1] = (int32_t)(f[i].y + (f[i + 1].y - f[i].y) * p);
    out[2] = (int32_t)(f[i].scale + (f[i + 1].scale - f[i].scale) * p);
    out[3] = (int32_t)(f[i].rot + (f[i + 1].rot - f[i].rot) * p);
}

/* ---------- Ring ---------- */

static int32_t gap_deg(void) { return ui.theme->style == BRUSHY_STYLE_BLOCKY ? 2 : 4; }
static int32_t seg_start(int z) { return 180 + z * 90 + gap_deg(); }
static int32_t seg_span(void) { return 90 - 2 * gap_deg(); }

static lv_obj_t * ring_arc(int32_t start, int32_t end, uint32_t color, lv_opa_t opa)
{
    bool rounded = ui.theme->style == BRUSHY_STYLE_ROUND;
    lv_obj_t * arc = lv_arc_create(ui.stage);
    lv_obj_remove_style_all(arc);
    lv_obj_remove_flag(arc, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(arc, RING_DIAM, RING_DIAM);
    lv_obj_center(arc);
    lv_arc_set_rotation(arc, 0);
    lv_arc_set_bg_angles(arc, start % 360, end % 360);
    lv_arc_set_angles(arc, start % 360, start % 360);
    for(int part = 0; part < 2; part++) {
        lv_style_selector_t sel = part ? LV_PART_INDICATOR : LV_PART_MAIN;
        lv_obj_set_style_arc_width(arc, RING_WIDTH, sel);
        lv_obj_set_style_arc_rounded(arc, rounded, sel);
    }
    lv_obj_set_style_arc_color(arc, lv_color_hex(color), LV_PART_MAIN);
    lv_obj_set_style_arc_opa(arc, opa, LV_PART_MAIN);
    lv_obj_set_style_arc_opa(arc, LV_OPA_TRANSP, LV_PART_INDICATOR);
    return arc;
}

static void build_ring(void)
{
    for(int z = 0; z < ZONES; z++) {
        ui.arcs[z] = ring_arc(seg_start(z), seg_start(z) + seg_span(), 0xffffff, 25);
        lv_obj_set_style_arc_color(ui.arcs[z], lv_color_hex(ui.theme->ring[z]), LV_PART_INDICATOR);
    }
    if(ui.theme->style == BRUSHY_STYLE_BLOCKY) {
        /* Black notches cut each zone into blocks, like the XP bar */
        for(int z = 0; z < ZONES; z++) {
            for(int k = 1; k < BLOCKS_PER_ZONE; k++) {
                int32_t a = seg_start(z) + k * seg_span() / BLOCKS_PER_ZONE;
                ring_arc(a, a + 1, 0x000000, LV_OPA_COVER);
            }
        }
    }
}

static void set_ring_progress(float t_ms)
{
    const float zone_ms = (float)TOTAL_MS / ZONES;
    for(int z = 0; z < ZONES; z++) {
        float p = (t_ms - z * zone_ms) / zone_ms;
        p = p < 0 ? 0 : (p > 1 ? 1 : p);
        int32_t start = seg_start(z);
        lv_arc_set_angles(ui.arcs[z], start % 360, (start + (int32_t)(p * seg_span())) % 360);
        lv_obj_set_style_arc_opa(ui.arcs[z], p > 0 ? LV_OPA_COVER : LV_OPA_TRANSP, LV_PART_INDICATOR);
    }
}

/* ---------- Countdown ---------- */

static lv_obj_t * row(lv_obj_t * parent, int32_t gap);

static void build_clock(void)
{
    ui.clock = row(ui.stage, 0);
    lv_obj_align(ui.clock, LV_ALIGN_TOP_MID, 0, CLOCK_Y - ui.theme->clock_digits[0]->header.h / 2);
    ui.clock_secs = -1;
}

static void clock_color(lv_color_t color)
{
    uint32_t n = lv_obj_get_child_count(ui.clock);
    for(uint32_t i = 0; i < n; i++) lv_obj_set_style_image_recolor(lv_obj_get_child(ui.clock, (int32_t)i), color, 0);
}

static void render_clock(float t_ms)
{
    int32_t secs = (int32_t)ceilf((TOTAL_MS - t_ms) / 1000.0f);
    if(secs == ui.clock_secs) return;
    ui.clock_secs = secs;

    const uint8_t glyphs[4] = {(uint8_t)(secs / 60), 10 /* ":" */, (uint8_t)(secs % 60 / 10), (uint8_t)(secs % 10)};
    lv_obj_clean(ui.clock);
    for(int i = 0; i < 4; i++) {
        lv_obj_t * img = ui_image(ui.clock, ui.theme->clock_digits[glyphs[i]]);
        lv_obj_set_style_image_opa(img, CLOCK_OPA, 0);
    }
    clock_color(lv_color_hex(ui.theme->ring[ui.zone < 0 ? 0 : ui.zone]));
}

static void set_active_zone(int zone)
{
    for(int z = 0; z < ZONES; z++) {
        lv_anim_delete(ui.arcs[z], exec_arc_opa);
        lv_obj_set_style_arc_opa(ui.arcs[z], 25, LV_PART_MAIN);
        if(z == zone) ui_anim_loop(ui.arcs[z], exec_arc_opa, 25, 90, 900, 0);
    }
    if(zone >= 0) clock_color(lv_color_hex(ui.theme->ring[zone]));
}

/* ---------- Buddy ---------- */

static void set_pose(brushy_pose_t pose)
{
    if(pose == ui.pose || !ui.theme->poses[pose]) return;
    ui.pose = pose;
    lv_image_set_src(ui.character, ui.theme->poses[pose]);
}

void ui_start_mood_motion(mood_t mood)
{
    lv_anim_delete(ui.character, NULL);
    lv_obj_set_style_translate_y(ui.character, 0, 0);
    lv_image_set_rotation(ui.character, 0);
    lv_image_set_scale(ui.character, 256);

    bool blocky = ui.theme->style == BRUSHY_STYLE_BLOCKY;
    switch(mood) {
        case MOOD_IDLE:
            ui_anim_loop(ui.character, exec_translate_y, 0, -5, blocky ? 600 : 1000, 0);
            break;
        case MOOD_SLEEPY:
            if(blocky) ui_anim_loop(ui.character, exec_translate_y, 0, -4, 1200, 0);
            else ui_anim_loop(ui.character, exec_breathe, 0, 1000, 1600, 0);
            break;
        case MOOD_BRUSHING:
            if(!blocky) ui_anim_loop(ui.character, ui_exec_rotation, -20, 20, 560, 0); /* pixel art stays upright */
            break;
        case MOOD_CHEER:
            ui_anim_loop(ui.character, exec_translate_y, 0, blocky ? -16 : -20, 350, 0);
            break;
    }
}

void ui_set_mood(mood_t mood)
{
    tricks_stop();
    ui_clear_pose_override();
    ui.mood = mood;
    ui.mood_since = lv_tick_get();
    ui_start_mood_motion(mood);
}

void ui_override_pose(brushy_pose_t pose, uint32_t ms)
{
    ui.pose_override = pose;
    ui.pose_override_until = ms ? lv_tick_get() + ms : 0;
}

void ui_clear_pose_override(void)
{
    ui.pose_override = BRUSHY_POSE_COUNT;
    ui.flying = false;
}

/* Picks the frame: an override (grin, fire, trick) or the mood's own (scrub, blink...). */
static void update_pose(uint32_t now)
{
    if(ui.pose_override != BRUSHY_POSE_COUNT && ui.pose_override_until &&
       (int32_t)(now - ui.pose_override_until) >= 0) {
        ui_clear_pose_override();
    }
    if(ui.flying) {
        set_pose((now / FLAP_MS) % 2 ? BRUSHY_POSE_FLY_B : BRUSHY_POSE_FLY_A);
        return;
    }
    if(ui.pose_override != BRUSHY_POSE_COUNT) {
        set_pose(ui.pose_override);
        return;
    }
    uint32_t t = now - ui.mood_since;
    switch(ui.mood) {
        case MOOD_IDLE:
            set_pose(t % BLINK_EVERY > BLINK_EVERY - BLINK_MS ? BRUSHY_POSE_BLINK : BRUSHY_POSE_IDLE);
            break;
        case MOOD_SLEEPY: set_pose(BRUSHY_POSE_SLEEPY); break;
        case MOOD_BRUSHING: set_pose((t / BRUSH_SWAP_MS) % 2 ? BRUSHY_POSE_BRUSH_B : BRUSHY_POSE_BRUSH_A); break;
        case MOOD_CHEER: set_pose(BRUSHY_POSE_CHEER); break;
    }
}

void ui_giggle(void)
{
    SOUND(SND_GIGGLE);
    if(tricks_active()) return; /* the trick is moving the buddy: just giggle out loud */
    lv_anim_delete(&ui.giggle_token, NULL);
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, &ui.giggle_token);
    lv_anim_set_exec_cb(&a, exec_giggle);
    lv_anim_set_values(&a, 0, 1000);
    lv_anim_set_duration(&a, 500);
    lv_anim_start(&a);
}

/* ---------- Overlays and confetti ---------- */

void ui_render_stars(bool bump, uint32_t value)
{
    char buf[12];
    lv_snprintf(buf, sizeof(buf), "%" LV_PRIu32, value);
    lv_obj_clean(ui.digits);
    for(const char * c = buf; *c; c++) ui_image(ui.digits, ui.theme->digits[*c - '0']);

    if(!bump) return;
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, ui.stars_row);
    lv_anim_set_exec_cb(&a, exec_scale);
    lv_anim_set_values(&a, 256, 410);
    lv_anim_set_duration(&a, 240);
    lv_anim_set_reverse_duration(&a, 360);
    lv_anim_start(&a);
}

static void confetti_done_cb(lv_anim_t * a) { lv_obj_delete(a->var); }

static void confetti(void)
{
    bool blocky = ui.theme->style == BRUSHY_STYLE_BLOCKY;
    for(int i = 0; i < CONFETTI; i++) {
        lv_obj_t * c = ui_plain_obj(ui.stage);
        uint32_t color = i % 5 == 4 ? 0xffffff : ui.theme->ring[i % 4];
        lv_obj_set_size(c, blocky ? 10 : 9, blocky ? 10 : 14);
        lv_obj_set_style_radius(c, blocky ? 0 : 3, 0);
        lv_obj_set_style_bg_color(c, lv_color_hex(color), 0);
        lv_obj_set_style_bg_opa(c, LV_OPA_COVER, 0);
        lv_obj_set_pos(c, (int32_t)lv_rand(0, SCREEN_SIZE - 10), -20);

        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, c);
        lv_anim_set_exec_cb(&a, exec_y);
        lv_anim_set_values(&a, -20, SCREEN_SIZE + 20);
        lv_anim_set_duration(&a, lv_rand(1800, 3400));
        lv_anim_set_delay(&a, lv_rand(0, 800));
        lv_anim_set_completed_cb(&a, confetti_done_cb);
        lv_anim_start(&a);
    }
}

static lv_obj_t * row(lv_obj_t * parent, int32_t gap)
{
    lv_obj_t * r = ui_plain_obj(parent);
    lv_obj_set_size(r, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(r, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(r, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(r, gap, 0);
    return r;
}

static void build_overlays(void)
{
    /* Reward counter (top) */
    ui.stars_row = row(ui.stage, 6);
    lv_obj_set_style_transform_pivot_x(ui.stars_row, LV_PCT(50), 0);
    lv_obj_set_style_transform_pivot_y(ui.stars_row, LV_PCT(50), 0);
    lv_obj_align(ui.stars_row, LV_ALIGN_TOP_MID, 0, 26);
    ui_image(ui.stars_row, ui.theme->reward);
    ui.digits = row(ui.stars_row, 1);

    /* Play hint (bottom) */
    ui.hint = ui_plain_obj(ui.stage);
    lv_obj_set_size(ui.hint, 56, 56);
    lv_obj_set_style_radius(ui.hint, ui.theme->style == BRUSHY_STYLE_BLOCKY ? 6 : LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(ui.hint, lv_color_hex(ui.theme->accent), 0);
    lv_obj_set_style_bg_opa(ui.hint, LV_OPA_COVER, 0);
    lv_obj_set_style_transform_pivot_x(ui.hint, 28, 0);
    lv_obj_set_style_transform_pivot_y(ui.hint, 28, 0);
    lv_obj_align(ui.hint, LV_ALIGN_BOTTOM_MID, 0, -36);
    lv_obj_t * play_icon = lv_label_create(ui.hint);
    lv_label_set_text(play_icon, LV_SYMBOL_PLAY);
    lv_obj_set_style_text_font(play_icon, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(play_icon, lv_color_black(), 0);
    lv_obj_align(play_icon, LV_ALIGN_CENTER, 2, 0);
    ui_anim_loop(ui.hint, exec_scale, 256, 282, 700, 0);

    /* "Bravo !" (bottom, done only) */
    ui.bravo = ui_image(ui.stage, ui.theme->bravo);
    lv_obj_set_style_transform_pivot_x(ui.bravo, LV_PCT(50), 0);
    lv_obj_set_style_transform_pivot_y(ui.bravo, LV_PCT(50), 0);
    lv_obj_align(ui.bravo, LV_ALIGN_BOTTOM_MID, 0, -44);
}

static void show(lv_obj_t * obj, bool visible)
{
    if(visible) lv_obj_remove_flag(obj, LV_OBJ_FLAG_HIDDEN);
    else lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
}

static void bravo_in(void)
{
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, ui.bravo);
    lv_anim_set_exec_cb(&a, exec_scale);
    lv_anim_set_values(&a, 100, 256);
    lv_anim_set_duration(&a, 600);
    lv_anim_set_path_cb(&a, lv_anim_path_overshoot);
    lv_anim_start(&a);
}

/* ---------- Phases ---------- */

static void set_phase(phase_t phase)
{
    tricks_stop();
    tricks_schedule();
    ui.phase = phase;
    ui.phase_since = lv_tick_get();
    show(ui.stars_row, phase != PHASE_BRUSHING);
    show(ui.clock, phase == PHASE_BRUSHING);
    show(ui.hint, phase == PHASE_IDLE);
    show(ui.bravo, phase == PHASE_DONE);
}

static void to_idle(void)
{
    set_phase(PHASE_IDLE);
    fx_clear();
    ui.elapsed_ms = 0;
    ui.zone = -1;
    fx_reset_germs();
    set_ring_progress(0);
    set_active_zone(-1);
    ui_set_mood(MOOD_IDLE);
    ui_render_stars(false, ui.stars);
}

static void start(void)
{
    MELODY(SND_START);
    set_phase(PHASE_BRUSHING);
    fx_clear();
    ui.elapsed_ms = 0;
    ui.zone = -1;
    ui.mirrored = false;
    fx_reset_germs();
    render_clock(0);
    ui_set_mood(MOOD_BRUSHING);
    ui_giggle();
}

static void finish(void)
{
    ui.elapsed_ms = TOTAL_MS;
    set_phase(PHASE_DONE);
    for(int i = 0; i < GERM_COUNT; i++) fx_pop_germ_quietly(&ui.germs[i]);
    set_ring_progress(TOTAL_MS);
    set_active_zone(-1);
    ui_set_mood(MOOD_CHEER);
    MELODY(SND_FINISH);
    ui.stars++;
    if(!ui.quiet) brushy_hal_save("stars", ui.stars); /* previews don't earn rewards */
    ui_render_stars(false, ui.stars - 1); /* the counter goes up when the reward lands */
    fx_reward_drop();
    bravo_in();
    confetti();
}

static void update_brushing(void)
{
    float t = ui.elapsed_ms < TOTAL_MS ? ui.elapsed_ms : TOTAL_MS;
    set_ring_progress(t);
    render_clock(t);

    int zone = (int)(t / (TOTAL_MS / ZONES));
    if(zone > ZONES - 1) zone = ZONES - 1;
    if(zone != ui.zone) {
        if(ui.zone != -1) {
            MELODY(SND_ZONE);
            ui_giggle();
        }
        ui.zone = zone;
        set_active_zone(zone);
    }

    for(int i = 0; i < GERM_COUNT; i++) {
        if(!ui.germs[i].popped && t >= ui.germs[i].pop_at_ms) fx_defeat_germ(&ui.germs[i]);
    }
    if(!ui.mirrored && t >= MIRROR_AT_MS) fx_mirror_check();

    if(ui.elapsed_ms >= TOTAL_MS) finish();
}

/* ---------- Theme ---------- */

/* Rebuilds every widget for `theme`. Only called on the start screen. */
static void load_theme(const brushy_theme_t * theme)
{
    tricks_stop();
    lv_anim_delete(&ui.giggle_token, NULL);
    lv_anim_delete(ui.stage, NULL);
    lv_obj_set_pos(ui.stage, 0, 0);
    fx_clear();
    lv_obj_clean(ui.stage); /* also stops animations of the deleted widgets */
    ui.theme = theme;
    ui.pose = BRUSHY_POSE_COUNT; /* force the next set_pose */

    build_ring();
    build_clock(); /* behind the germs and the buddy */
    fx_build_germs();
    ui.character = ui_image(ui.stage, theme->poses[BRUSHY_POSE_IDLE]);
    lv_obj_set_size(ui.character, CHAR_SIZE, CHAR_SIZE);
    lv_obj_center(ui.character);
    lv_image_set_pivot(ui.character, CHAR_SIZE / 2, CHAR_SIZE); /* bottom centre: feet stay planted */
    build_overlays();

    to_idle();
    update_pose(lv_tick_get());

    for(uint32_t i = 0; i < brushy_theme_count; i++) {
        if(brushy_themes[i] == theme) brushy_hal_save("theme", i);
    }
}

static void next_theme(void)
{
    uint32_t i = 0;
    while(i < brushy_theme_count && brushy_themes[i] != ui.theme) i++;
    load_theme(brushy_themes[(i + 1) % brushy_theme_count]);
    ui_giggle();
}

/* ---------- Input and main loop ---------- */

static void on_input(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch(ui.phase) {
        case PHASE_IDLE: /* tap = start, press and hold = next theme */
            if(code == LV_EVENT_SHORT_CLICKED) start();
            else if(code == LV_EVENT_LONG_PRESSED) next_theme();
            break;
        case PHASE_BRUSHING:
            if(code == LV_EVENT_PRESSED) ui_giggle();
            break;
        case PHASE_DONE:
            if(code == LV_EVENT_PRESSED && lv_tick_elaps(ui.phase_since) > DONE_TAP_GUARD_MS) to_idle();
            break;
    }
}

static void loop_cb(lv_timer_t * t)
{
    LV_UNUSED(t);
    uint32_t now = lv_tick_get();
    uint32_t dt = now - ui.last_tick;
    ui.last_tick = now;
    float in_phase = (float)(now - ui.phase_since) * ui.speed;

    switch(ui.phase) {
        case PHASE_BRUSHING:
            ui.elapsed_ms += dt * ui.speed;
            update_brushing();
            break;
        case PHASE_IDLE:
            if(ui.mood == MOOD_IDLE && in_phase > SLEEP_AFTER_MS) ui_set_mood(MOOD_SLEEPY);
            break;
        case PHASE_DONE:
            if(in_phase > CELEBRATE_MS) to_idle();
            break;
    }
    tricks_update(now);
    update_pose(now);
}

/* ---------- Public API ---------- */

void brushy_ui_create(lv_obj_t * screen)
{
    memset(&ui, 0, sizeof(ui));
    ui.screen = screen;
    ui.speed = 1;
    ui.pose_override = BRUSHY_POSE_COUNT;
    ui.stars = brushy_hal_load("stars", 0);

    lv_obj_set_style_bg_color(screen, lv_color_black(), 0); /* AMOLED: black = pixels off */
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    lv_obj_remove_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

    /* Full-screen, clickable: receives every tap (nothing inside is clickable). */
    ui.stage = lv_obj_create(screen);
    lv_obj_remove_style_all(ui.stage);
    lv_obj_remove_flag(ui.stage, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(ui.stage, SCREEN_SIZE, SCREEN_SIZE);
    lv_obj_add_event_cb(ui.stage, on_input, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui.stage, on_input, LV_EVENT_SHORT_CLICKED, NULL);
    lv_obj_add_event_cb(ui.stage, on_input, LV_EVENT_LONG_PRESSED, NULL);
    for(lv_indev_t * in = lv_indev_get_next(NULL); in; in = lv_indev_get_next(in)) {
        lv_indev_set_long_press_time(in, LONG_PRESS_MS);
    }

    uint32_t saved = brushy_hal_load("theme", 0);
    load_theme(brushy_themes[saved < brushy_theme_count ? saved : 0]);

    ui.last_tick = lv_tick_get();
    lv_timer_create(loop_cb, LOOP_MS, NULL);
}

bool brushy_ui_set_theme(const char * id)
{
    for(uint32_t i = 0; i < brushy_theme_count; i++) {
        if(strcmp(brushy_themes[i]->id, id) == 0) {
            load_theme(brushy_themes[i]);
            return true;
        }
    }
    return false;
}

bool brushy_ui_trick(const char * name)
{
    static const char * const names[TRICK_COUNT] = {"peek", "peek_side", "slide", "mine", "rainbow", "fly"};
    for(int i = 0; i < TRICK_COUNT; i++) {
        if(strcmp(name, names[i]) == 0) return tricks_start((trick_t)i);
    }
    if(strcmp(name, "mirror") == 0) fx_mirror_check();
    else if(strcmp(name, "shake") == 0) fx_shake();
    else return false;
    return true;
}

void brushy_ui_set_speed(float speed)
{
    ui.speed = speed > 0 ? speed : 1;
}

void brushy_ui_preview(const char * state, float seconds)
{
    ui.quiet = true;
    if(strcmp(state, "sleepy") == 0) {
        ui_set_mood(MOOD_SLEEPY);
    }
    else if(strcmp(state, "brushing") == 0) {
        start();
        ui.elapsed_ms = seconds * 1000;
        for(int i = 0; i < GERM_COUNT; i++) { /* skip the effects of earlier germs */
            if(ui.elapsed_ms >= ui.germs[i].pop_at_ms) fx_pop_germ_quietly(&ui.germs[i]);
        }
        ui.mirrored = ui.elapsed_ms >= MIRROR_AT_MS;
        update_brushing();
    }
    else if(strcmp(state, "done") == 0) {
        start();
        finish();
    }
    update_pose(lv_tick_get());
    ui.quiet = false;
}
