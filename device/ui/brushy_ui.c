/**
 * Brushy Buddy UI. Mirrors the behaviour of the browser prototype (prototype/js/app.js):
 *   idle     - buddy waits (falls asleep after 30 s); tap to start,
 *              press and hold to switch theme (when built with more than one)
 *   brushing - 2 minutes, 4 zones of 30 s filling the ring clockwise from top-left;
 *              germs are defeated over time (see brushy_fx.c), tap = giggle
 *   done     - cheer, confetti, "Bravo !", the reward drops and is collected;
 *              back to idle on tap or after 12 s
 * While brushing, the buddy plays a random trick every 8-15 s (see brushy_tricks.c).
 */
#include "brushy_ui_priv.h"

#include "src/draw/lv_image_decoder_private.h" /* lv_image_decoder_dsc_t, to preload images */
#include "src/misc/lv_area_private.h"           /* lv_area_intersect */

#include <math.h>
#include <string.h>

#define SLEEP_AFTER_MS    30000
#define CELEBRATE_MS      12000
#define DONE_TAP_GUARD_MS 1500
#define LONG_PRESS_MS     900

#define RING_RADIUS     224 /* outer edge; 18 px stroke centred on r = 215 */
#define RING_WIDTH      18
#define TRACK_OPA       25
#define BLOCKS_PER_ZONE 8   /* blocky style */
#define CONFETTI        50

#define CLOCK_Y       88 /* centre of the countdown: the free band above the buddy's head */
#define CLOCK_OPA     82 /* about 32 % */

#define BATTERY_WARN_MS 5000
#define BATTERY_FADE_MS 400
#define BATTERY_RED     0xff3b30

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
static const brushy_note_t SND_LOW_BATTERY[] = {
    {660, 0, 140, BRUSHY_WAVE_TRIANGLE, 50}, {0, 0, 40, BRUSHY_WAVE_TRIANGLE, 0}, {440, 330, 300, BRUSHY_WAVE_TRIANGLE, 50},
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
static void exec_scale(void * obj, int32_t v) { lv_obj_set_style_transform_scale(obj, v, 0); }
static void exec_y(void * obj, int32_t v) { lv_obj_set_y(obj, v); }

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

/* The ring is one object that draws its arcs itself. LVGL draws an arc by masking every pixel
 * of its 448 px square, so as separate lv_arc widgets the arcs made every redraw anywhere on
 * screen expensive, and restyling one (the pulse) redrew the whole screen. Here, areas inside
 * the ring skip the arcs, and changes only redraw the stretch of ring they affect. */

/* Screen position of the ring's top-left corner (the stage moves when it shakes) */
static lv_point_t ring_origin(void)
{
    lv_area_t c;
    lv_obj_get_coords(ui.ring, &c);
    return (lv_point_t){c.x1, c.y1};
}

/* True if `a` (relative to the ring) overlaps the band between its inner and outer edges */
static bool ring_band_overlaps(const lv_area_t * a)
{
    int32_t near_x = LV_CLAMP(a->x1, CENTER, a->x2) - CENTER, near_y = LV_CLAMP(a->y1, CENTER, a->y2) - CENTER;
    int32_t far_x = LV_MAX(LV_ABS(a->x1 - CENTER), LV_ABS(a->x2 - CENTER));
    int32_t far_y = LV_MAX(LV_ABS(a->y1 - CENTER), LV_ABS(a->y2 - CENTER));
    int32_t r_in = RING_RADIUS - RING_WIDTH - 1, r_out = RING_RADIUS + 1;
    return near_x * near_x + near_y * near_y <= r_out * r_out && far_x * far_x + far_y * far_y >= r_in * r_in;
}

/* Bounding box on screen of the ring between angles `from` and `to` (degrees, at most 90 apart
 * and not across a multiple of 90, like the zones) */
static void ring_area(int32_t from, int32_t to, lv_area_t * area)
{
    const int32_t r = RING_RADIUS - RING_WIDTH / 2, pad = RING_WIDTH / 2 + 2; /* + rounded caps */
    lv_point_t o = ring_origin();
    *area = (lv_area_t){INT16_MAX, INT16_MAX, INT16_MIN, INT16_MIN};
    for(int32_t a = from; ; a = LV_MIN(a + 10, to)) {
        int32_t x = o.x + CENTER + lv_trigo_cos(a) * r / LV_TRIGO_SIN_MAX;
        int32_t y = o.y + CENTER + lv_trigo_sin(a) * r / LV_TRIGO_SIN_MAX;
        area->x1 = LV_MIN(area->x1, x - pad);
        area->y1 = LV_MIN(area->y1, y - pad);
        area->x2 = LV_MAX(area->x2, x + pad);
        area->y2 = LV_MAX(area->y2, y + pad);
        if(a == to) break;
    }
}

/* LVGL masks every pixel of the arc's square that is being redrawn, so the arc is clipped
 * to its own bounding box first: a 1 degree notch then costs almost nothing */
static void draw_ring_arc(lv_layer_t * layer, int32_t start, int32_t end, lv_color_t color, lv_opa_t opa)
{
    lv_area_t box, clip_saved = layer->_clip_area;
    ring_area(start, end, &box);
    if(!lv_area_intersect(&layer->_clip_area, &clip_saved, &box)) {
        layer->_clip_area = clip_saved;
        return;
    }
    lv_point_t o = ring_origin();
    lv_draw_arc_dsc_t dsc;
    lv_draw_arc_dsc_init(&dsc);
    dsc.center.x = o.x + CENTER;
    dsc.center.y = o.y + CENTER;
    dsc.radius = RING_RADIUS;
    dsc.width = RING_WIDTH;
    dsc.rounded = ui.theme->style == BRUSHY_STYLE_ROUND;
    dsc.start_angle = start % 360;
    dsc.end_angle = end % 360;
    dsc.color = color;
    dsc.opa = opa;
    lv_draw_arc(layer, &dsc);
    layer->_clip_area = clip_saved;
}

static void ring_draw_cb(lv_event_t * e)
{
    lv_layer_t * layer = lv_event_get_layer(e);
    lv_point_t o = ring_origin();
    lv_area_t clip = layer->_clip_area;
    lv_area_move(&clip, -o.x, -o.y);
    if(!ring_band_overlaps(&clip)) return;

    for(int z = 0; z < ZONES; z++) {
        int32_t start = seg_start(z);
        draw_ring_arc(layer, start, start + seg_span(), lv_color_white(), ui.ring_track_opa[z]);
        if(ui.ring_fill[z] > 0) {
            draw_ring_arc(layer, start, start + ui.ring_fill[z], lv_color_hex(ui.theme->ring[z]), LV_OPA_COVER);
        }
    }
    if(ui.theme->style == BRUSHY_STYLE_BLOCKY) {
        /* Black notches cut each zone into blocks, like the XP bar */
        for(int z = 0; z < ZONES; z++) {
            for(int k = 1; k < BLOCKS_PER_ZONE; k++) {
                int32_t a = seg_start(z) + k * seg_span() / BLOCKS_PER_ZONE;
                draw_ring_arc(layer, a, a + 1, lv_color_black(), LV_OPA_COVER);
            }
        }
    }
}

/* Redraws the ring between angles `from` and `to` (degrees), in pieces of 10 degrees or less
 * so that only the band is redrawn, not the square around it */
static void ring_invalidate(int32_t from, int32_t to)
{
    for(int32_t a = from; a < to; a += 10) {
        lv_area_t area;
        ring_area(a, LV_MIN(a + 10, to), &area);
        lv_obj_invalidate_area(ui.ring, &area);
    }
}

static void build_ring(void)
{
    ui.ring = ui_plain_obj(ui.stage);
    lv_obj_set_size(ui.ring, SCREEN_SIZE, SCREEN_SIZE);
    lv_obj_add_event_cb(ui.ring, ring_draw_cb, LV_EVENT_DRAW_MAIN, NULL);
    for(int z = 0; z < ZONES; z++) {
        ui.ring_fill[z] = 0;
        ui.ring_track_opa[z] = TRACK_OPA;
    }
}

static void set_ring_progress(float t_ms)
{
    const float zone_ms = (float)TOTAL_MS / ZONES;
    for(int z = 0; z < ZONES; z++) {
        float p = (t_ms - z * zone_ms) / zone_ms;
        p = p < 0 ? 0 : (p > 1 ? 1 : p);
        int32_t fill = (int32_t)(p * seg_span());
        if(fill == ui.ring_fill[z]) continue;
        int32_t start = seg_start(z);
        ring_invalidate(start + LV_MIN(fill, ui.ring_fill[z]), start + LV_MAX(fill, ui.ring_fill[z]) + 1);
        ui.ring_fill[z] = fill;
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

static void set_track_opa(int zone, lv_opa_t opa)
{
    if(opa == ui.ring_track_opa[zone]) return;
    ui.ring_track_opa[zone] = opa;
    ring_invalidate(seg_start(zone), seg_start(zone) + seg_span());
}

/* The active zone's track pulses; the animation's var is the zone's ring_track_opa entry */
static void exec_track_pulse(void * var, int32_t v)
{
    set_track_opa((int)((lv_opa_t *)var - ui.ring_track_opa), (lv_opa_t)v);
}

static void set_active_zone(int zone)
{
    for(int z = 0; z < ZONES; z++) {
        lv_anim_delete(&ui.ring_track_opa[z], exec_track_pulse);
        set_track_opa(z, TRACK_OPA);
        if(z == zone) ui_anim_loop(&ui.ring_track_opa[z], exec_track_pulse, TRACK_OPA, 90, 900, 0);
    }
    if(zone >= 0) clock_color(lv_color_hex(ui.theme->ring[zone]));
}

/* ---------- Buddy ---------- */

/* Index of the pre-zoomed peek-a-boo frame matching where tricks put the buddy, or -1 */
static int peek_tilt(void)
{
    if(ui.buddy_scale != (uint32_t)(BRUSHY_PEEK_ZOOM * 256)) return -1;
    for(int i = 0; i < BRUSHY_PEEK_TILTS; i++) {
        if(ui.buddy_rot == (i - 1) * BRUSHY_PEEK_TILT * 10) return ui.theme->peek[ui.pose][i] ? i : -1;
    }
    return -1;
}

/* Like lv_obj_set_size and lv_obj_align, but only redraws when something changes:
 * tricks place the buddy on every frame, even while he holds still */
static void set_box(lv_obj_t * obj, int32_t w, int32_t h, lv_align_t align, int32_t x, int32_t y)
{
    if(lv_obj_get_style_width(obj, 0) != w || lv_obj_get_style_height(obj, 0) != h) lv_obj_set_size(obj, w, h);
    if(lv_obj_get_style_align(obj, 0) != align || lv_obj_get_style_x(obj, 0) != x || lv_obj_get_style_y(obj, 0) != y) {
        lv_obj_align(obj, align, x, y);
    }
}

/* Puts the buddy where tricks asked (ui.buddy_*): a pre-zoomed frame placed by its feet,
 * or the pose scaled and rotated by LVGL */
static void place_character(int tilt)
{
    if(tilt >= 0) {
        const lv_image_dsc_t * img = ui.theme->peek[ui.pose][tilt];
        const lv_point_t * feet = ui.theme->peek_feet[ui.pose][tilt];
        set_box(ui.character, img->header.w, img->header.h, LV_ALIGN_TOP_LEFT,
                CENTER + ui.buddy_x - feet->x, CENTER + CHAR_SIZE / 2 + ui.buddy_y - feet->y);
        lv_image_set_scale(ui.character, 256);
        lv_image_set_rotation(ui.character, 0);
    }
    else {
        set_box(ui.character, CHAR_SIZE, CHAR_SIZE, LV_ALIGN_CENTER, ui.buddy_x, ui.buddy_y);
        lv_image_set_scale(ui.character, ui.buddy_scale);
        lv_image_set_rotation(ui.character, ui.buddy_rot);
    }
    ui.peeking = tilt >= 0;
}

/* Shows the current pose: pre-zoomed while peeking, tilted by the brushing sway if it has
 * pre-rotated frames */
static void show_pose(void)
{
    if(ui.pose >= BRUSHY_POSE_COUNT) return;
    int tilt = peek_tilt();
    const lv_image_dsc_t * src = ui.theme->poses[ui.pose];
    if(tilt >= 0) {
        src = ui.theme->peek[ui.pose][tilt];
    }
    else if(ui.sway_tilt >= 0 && (ui.pose == BRUSHY_POSE_BRUSH_A || ui.pose == BRUSHY_POSE_BRUSH_B)) {
        src = ui.theme->brush_sway[ui.pose - BRUSHY_POSE_BRUSH_A][ui.sway_tilt];
    }
    if(lv_image_get_src(ui.character) != src) lv_image_set_src(ui.character, src);
    if(ui.peeking || tilt >= 0) place_character(tilt); /* entering, leaving or another frame */
}

/* Decompresses the peek-a-boo frames now (~30 ms each) rather than when the trick first shows
 * them: they stay in LVGL's image cache (LV_CACHE_DEF_SIZE must hold them all) */
static void preload_peek_frames(void)
{
    for(int pose = 0; pose < BRUSHY_POSE_COUNT; pose++) {
        for(int i = 0; i < BRUSHY_PEEK_TILTS; i++) {
            lv_image_decoder_dsc_t dsc;
            if(ui.theme->peek[pose][i] && lv_image_decoder_open(&dsc, ui.theme->peek[pose][i], NULL) == LV_RESULT_OK) {
                lv_image_decoder_close(&dsc);
            }
        }
    }
}

void ui_place_buddy(int32_t x, int32_t y, uint32_t scale, int32_t rot)
{
    ui.buddy_x = x;
    ui.buddy_y = y;
    ui.buddy_scale = scale;
    ui.buddy_rot = rot;
    show_pose(); /* places the buddy if it shows a peek frame */
    if(!ui.peeking) place_character(-1);
}

static void set_pose(brushy_pose_t pose)
{
    if(pose == ui.pose || !ui.theme->poses[pose]) return;
    ui.pose = pose;
    show_pose();
}

/* v: -20..20 (0.1 degree) -> the nearest pre-rotated brushing frame */
static void exec_sway(void * obj, int32_t v)
{
    LV_UNUSED(obj);
    ui.sway_tilt = (int8_t)LV_CLAMP(0, (v + 20 + 5) / 10, BRUSHY_SWAY_TILTS - 1);
    show_pose();
}

void ui_stop_mood_motion(void)
{
    lv_anim_delete(ui.character, NULL);
    lv_obj_set_style_translate_y(ui.character, 0, 0);
    ui.sway_tilt = -1;
    show_pose();
}

void ui_start_mood_motion(mood_t mood)
{
    ui_stop_mood_motion();
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
            if(ui.theme->brush_sway[0][0]) ui_anim_loop(ui.character, exec_sway, -20, 20, 560, 0); /* pixel art stays upright */
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

/* Picks the frame: an override (grin, spit, trick) or the mood's own (scrub, blink...). */
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

/* ---------- Low battery warning ---------- */

static void exec_opa(void * obj, int32_t v) { lv_obj_set_style_opa(obj, (lv_opa_t)v, 0); }
static void exec_bg_opa(void * obj, int32_t v) { lv_obj_set_style_bg_opa(obj, (lv_opa_t)v, 0); }
static void delete_done_cb(lv_anim_t * a) { lv_obj_delete(a->var); }

static void battery_warning_close(lv_obj_t * warning)
{
    if(lv_anim_get(warning, exec_opa)) return; /* already fading out */
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, warning);
    lv_anim_set_exec_cb(&a, exec_opa);
    lv_anim_set_values(&a, LV_OPA_COVER, LV_OPA_TRANSP);
    lv_anim_set_duration(&a, BATTERY_FADE_MS);
    lv_anim_set_completed_cb(&a, delete_done_cb);
    lv_anim_start(&a);
}

static void battery_warning_tap_cb(lv_event_t * e) { battery_warning_close(lv_event_get_target(e)); }
static void battery_warning_timer_cb(lv_timer_t * t) { battery_warning_close(lv_timer_get_user_data(t)); }

/* A big battery with a blinking red sliver and the percentage, over the dimmed buddy. It sits on
 * the screen above the stage, so it swallows taps and survives a theme change. */
static void battery_warning(int percent)
{
    lv_obj_t * warning = lv_obj_create(ui.screen);
    lv_obj_remove_style_all(warning);
    lv_obj_remove_flag(warning, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(warning, SCREEN_SIZE, SCREEN_SIZE);
    lv_obj_set_style_bg_color(warning, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(warning, LV_OPA_80, 0);
    lv_obj_add_event_cb(warning, battery_warning_tap_cb, LV_EVENT_CLICKED, NULL);

    const int32_t w = 170, h = 86, border = 8, pad = 6, nub_w = 14, nub_h = 36;
    lv_obj_t * body = ui_plain_obj(warning);
    lv_obj_set_size(body, w, h);
    lv_obj_align(body, LV_ALIGN_CENTER, -nub_w / 2, -30);
    lv_obj_set_style_radius(body, 14, 0);
    lv_obj_set_style_border_width(body, border, 0);
    lv_obj_set_style_border_color(body, lv_color_white(), 0);
    lv_obj_set_style_border_opa(body, LV_OPA_COVER, 0);

    lv_obj_t * nub = ui_plain_obj(warning);
    lv_obj_set_size(nub, nub_w, nub_h);
    lv_obj_align_to(nub, body, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
    lv_obj_set_style_radius(nub, 5, 0);
    lv_obj_set_style_bg_color(nub, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(nub, LV_OPA_COVER, 0);

    const int32_t inside = w - 2 * (border + pad);
    lv_obj_t * fill = ui_plain_obj(body);
    lv_obj_set_size(fill, LV_MAX(inside * percent / 100, 12), h - 2 * (border + pad));
    lv_obj_align(fill, LV_ALIGN_LEFT_MID, pad, 0);
    lv_obj_set_style_radius(fill, 5, 0);
    lv_obj_set_style_bg_color(fill, lv_color_hex(BATTERY_RED), 0);
    lv_obj_set_style_bg_opa(fill, LV_OPA_COVER, 0);
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, fill);
    lv_anim_set_exec_cb(&a, exec_bg_opa);
    lv_anim_set_values(&a, LV_OPA_COVER, LV_OPA_20);
    lv_anim_set_duration(&a, 450);
    lv_anim_set_reverse_duration(&a, 450);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a);

    lv_obj_t * label = lv_label_create(warning);
    lv_label_set_text_fmt(label, "%d%%", percent);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_40, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(BATTERY_RED), 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 60);

    lv_timer_t * t = lv_timer_create(battery_warning_timer_cb, BATTERY_WARN_MS, warning);
    lv_timer_set_repeat_count(t, 1);
    SOUND(SND_LOW_BATTERY);
}

/* ---------- Phases ---------- */

static void set_phase(phase_t phase)
{
    tricks_stop();
    tricks_schedule();
    ui.phase = phase;
    ui.phase_since = lv_tick_get();
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
}

static void start(void)
{
    MELODY(SND_START);
    set_phase(PHASE_BRUSHING);
    fx_clear();
    ui.elapsed_ms = 0;
    ui.zone = -1;
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
    ui.buddy_x = ui.buddy_y = ui.buddy_rot = 0;
    ui.buddy_scale = 256;
    ui.peeking = false;
    preload_peek_frames();
    build_overlays();

    to_idle();
    update_pose(lv_tick_get());

    for(uint32_t i = 0; i < brushy_theme_count; i++) {
        if(brushy_themes[i] == theme) brushy_hal_save("theme", i);
    }
}

static void next_theme(void)
{
    if(brushy_theme_count < 2) return; /* a device built for one theme */
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

void brushy_ui_battery(int percent)
{
    if(percent >= 0 && percent <= BRUSHY_LOW_BATTERY) battery_warning(percent);
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
    static const char * const names[TRICK_COUNT] = {"peek", "peek_side", "slide", "mine", "rainbow", "fly", "xp"};
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
        update_brushing();
    }
    else if(strcmp(state, "done") == 0) {
        start();
        finish();
    }
    update_pose(lv_tick_get());
    ui.quiet = false;
}
