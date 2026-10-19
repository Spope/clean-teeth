/**
 * Brushy Buddy UI. Mirrors the behaviour of the browser prototype (prototype/js/app.js):
 *   idle     - buddy waits (falls asleep after 30 s), tap anywhere to start
 *   brushing - 2 minutes, 4 zones of 30 s filling the ring clockwise from top-left,
 *              germs pop over time, tap = giggle
 *   done     - cheer, confetti, "Bravo!", +1 star; back to idle on tap or after 12 s
 */
#include "brushy_ui.h"
#include "brushy_hal.h"
#include "assets.h"

#include <math.h>
#include <string.h>

#define TOTAL_MS          120000
#define ZONES             4
#define GERMS_PER_ZONE    3
#define GERM_COUNT        (ZONES * GERMS_PER_ZONE)
#define SLEEP_AFTER_MS    30000
#define CELEBRATE_MS      12000
#define DONE_TAP_GUARD_MS 1500

#define SCREEN_SIZE  466
#define CENTER       (SCREEN_SIZE / 2)
#define RING_DIAM    448 /* 18 px stroke centred on r = 215 */
#define RING_WIDTH   18
#define RING_GAP_DEG 4   /* half the gap between segments, leaves room for round caps */
#define GERM_RADIUS  168
#define GERM_SIZE    52
#define CHAR_SIZE    250
#define CONFETTI     50

#define LOOP_MS       33
#define BRUSH_SWAP_MS 280
#define BLINK_EVERY   4000
#define BLINK_MS      150

#define STAR_COLOR 0xffd166

typedef enum { PHASE_IDLE, PHASE_BRUSHING, PHASE_DONE } phase_t;
typedef enum { MOOD_IDLE, MOOD_SLEEPY, MOOD_BRUSHING, MOOD_CHEER } mood_t;

typedef struct {
    lv_obj_t * img;
    uint32_t pop_at_ms;
    bool popped;
} germ_t;

static struct {
    const brushy_character_t * ch;
    lv_obj_t * screen;
    lv_obj_t * character;
    lv_obj_t * arcs[ZONES];
    germ_t germs[GERM_COUNT];
    lv_obj_t * stars_row;
    lv_obj_t * stars_label;
    lv_obj_t * hint;
    lv_obj_t * bravo;

    phase_t phase;
    mood_t mood;
    brushy_pose_t pose;
    float speed;
    float elapsed_ms;
    int zone;
    uint32_t phase_since;
    uint32_t mood_since;
    uint32_t last_tick;
    uint32_t stars;
    bool quiet; /* no sounds while jumping to a preview state */
    int giggle_token; /* address used as the giggle animation's var */
} ui;

/* ---------- Sounds (same melodies as prototype/js/sound.js) ---------- */

#define NOTE(f, ms) {f, 0, ms, BRUSHY_WAVE_TRIANGLE, 60}

static const brushy_note_t SND_START[] = {NOTE(523, 100), NOTE(659, 100), NOTE(784, 200)};
static const brushy_note_t SND_ZONE[] = {NOTE(784, 120), NOTE(1047, 220)};
static const brushy_note_t SND_POP[] = {{500, 1400, 120, BRUSHY_WAVE_SINE, 80}};
static const brushy_note_t SND_GIGGLE[] = {
    {880, 0, 60, BRUSHY_WAVE_SQUARE, 20}, {988, 0, 60, BRUSHY_WAVE_SQUARE, 20},
    {880, 0, 60, BRUSHY_WAVE_SQUARE, 20}, {1175, 0, 100, BRUSHY_WAVE_SQUARE, 20},
};
static const brushy_note_t SND_FINISH[] = {
    NOTE(523, 120), NOTE(659, 120), NOTE(784, 120), NOTE(1047, 240),
    NOTE(0, 60), NOTE(784, 120), NOTE(1047, 400),
};

#define PLAY(snd) do { if(!ui.quiet) brushy_hal_play(snd, sizeof(snd) / sizeof(snd[0])); } while(0)

/* ---------- Animation helpers ---------- */

static void exec_translate_y(void * obj, int32_t v) { lv_obj_set_style_translate_y(obj, v, 0); }
static void exec_rotation(void * obj, int32_t v) { lv_image_set_rotation(obj, v); }
static void exec_arc_opa(void * obj, int32_t v) { lv_obj_set_style_arc_opa(obj, (lv_opa_t)v, LV_PART_MAIN); }
static void exec_scale(void * obj, int32_t v) { lv_obj_set_style_transform_scale(obj, v, 0); }
static void exec_y(void * obj, int32_t v) { lv_obj_set_y(obj, v); }

/* v: 0..1000 -> slightly wider and shorter, like breathing */
static void exec_breathe(void * obj, int32_t v)
{
    lv_image_set_scale_x(obj, 256 + 8 * v / 1000);
    lv_image_set_scale_y(obj, 256 - 5 * v / 1000);
}

/* v: 0..1000 -> grow to 1.5x, then shrink to nothing */
static void exec_germ_pop(void * obj, int32_t v)
{
    int32_t s = v < 400 ? 256 + 128 * v / 400 : 384 * (1000 - v) / 600;
    lv_image_set_scale(obj, (uint32_t)LV_MAX(s, 1));
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

static void anim_loop(void * var, lv_anim_exec_xcb_t cb, int32_t from, int32_t to, uint32_t ms, uint32_t delay)
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
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_start(&a);
}

static lv_obj_t * plain_obj(lv_obj_t * parent)
{
    lv_obj_t * o = lv_obj_create(parent);
    lv_obj_remove_style_all(o);
    lv_obj_remove_flag(o, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    return o;
}

/* ---------- Ring ---------- */

static int32_t seg_start(int z) { return 180 + z * 90 + RING_GAP_DEG; }
#define SEG_SPAN (90 - 2 * RING_GAP_DEG)

static void build_ring(void)
{
    for(int z = 0; z < ZONES; z++) {
        lv_obj_t * arc = lv_arc_create(ui.screen);
        lv_obj_remove_style_all(arc);
        lv_obj_remove_flag(arc, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_size(arc, RING_DIAM, RING_DIAM);
        lv_obj_center(arc);
        lv_arc_set_rotation(arc, 0);
        lv_arc_set_bg_angles(arc, seg_start(z) % 360, (seg_start(z) + SEG_SPAN) % 360);
        lv_arc_set_angles(arc, seg_start(z) % 360, seg_start(z) % 360);

        for(int part = 0; part < 2; part++) {
            lv_style_selector_t sel = part ? LV_PART_INDICATOR : LV_PART_MAIN;
            lv_obj_set_style_arc_width(arc, RING_WIDTH, sel);
            lv_obj_set_style_arc_rounded(arc, true, sel);
        }
        lv_obj_set_style_arc_color(arc, lv_color_white(), LV_PART_MAIN);
        lv_obj_set_style_arc_opa(arc, 25, LV_PART_MAIN);
        lv_obj_set_style_arc_color(arc, lv_color_hex(ui.ch->ring[z]), LV_PART_INDICATOR);
        lv_obj_set_style_arc_opa(arc, LV_OPA_TRANSP, LV_PART_INDICATOR);
        ui.arcs[z] = arc;
    }
}

static void set_ring_progress(float t_ms)
{
    const float zone_ms = (float)TOTAL_MS / ZONES;
    for(int z = 0; z < ZONES; z++) {
        float p = (t_ms - z * zone_ms) / zone_ms;
        p = p < 0 ? 0 : (p > 1 ? 1 : p);
        int32_t start = seg_start(z);
        lv_arc_set_angles(ui.arcs[z], start % 360, (start + (int32_t)(p * SEG_SPAN)) % 360);
        lv_obj_set_style_arc_opa(ui.arcs[z], p > 0 ? LV_OPA_COVER : LV_OPA_TRANSP, LV_PART_INDICATOR);
    }
}

static void set_active_zone(int zone)
{
    for(int z = 0; z < ZONES; z++) {
        lv_anim_delete(ui.arcs[z], exec_arc_opa);
        lv_obj_set_style_arc_opa(ui.arcs[z], 25, LV_PART_MAIN);
        if(z == zone) anim_loop(ui.arcs[z], exec_arc_opa, 25, 90, 900, 0);
    }
}

/* ---------- Germs ---------- */

static const lv_image_dsc_t * const GERM_IMAGES[] = {&img_germ_0, &img_germ_1, &img_germ_2, &img_germ_3};

static void germ_wobble(germ_t * g, uint32_t delay)
{
    anim_loop(g->img, exec_rotation, -100, 100, 1400, delay);
}

static void build_germs(void)
{
    const float zone_ms = (float)TOTAL_MS / ZONES;
    for(int z = 0; z < ZONES; z++) {
        for(int i = 0; i < GERMS_PER_ZONE; i++) {
            float deg = 180.0f + z * 90.0f + 90.0f * (i + 1) / (GERMS_PER_ZONE + 1);
            float rad = deg * 3.14159265f / 180.0f;
            germ_t * g = &ui.germs[z * GERMS_PER_ZONE + i];
            g->img = lv_image_create(ui.screen);
            lv_image_set_src(g->img, GERM_IMAGES[(z + i) % 4]);
            lv_image_set_pivot(g->img, GERM_SIZE / 2, GERM_SIZE / 2);
            lv_obj_set_pos(g->img, (int32_t)(CENTER + GERM_RADIUS * cosf(rad)) - GERM_SIZE / 2,
                           (int32_t)(CENTER + GERM_RADIUS * sinf(rad)) - GERM_SIZE / 2);
            g->pop_at_ms = (uint32_t)(z * zone_ms + (i + 1) * zone_ms / GERMS_PER_ZONE);
        }
    }
}

static void reset_germs(void)
{
    for(int i = 0; i < GERM_COUNT; i++) {
        germ_t * g = &ui.germs[i];
        lv_anim_delete(g->img, NULL);
        g->popped = false;
        lv_obj_remove_flag(g->img, LV_OBJ_FLAG_HIDDEN);
        lv_image_set_scale(g->img, 256);
        germ_wobble(g, (uint32_t)(i * 230 % 1400));
    }
}

static void germ_popped_cb(lv_anim_t * a) { lv_obj_add_flag(a->var, LV_OBJ_FLAG_HIDDEN); }

static void pop_germ(germ_t * g, bool sound)
{
    g->popped = true;
    lv_anim_delete(g->img, NULL);
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, g->img);
    lv_anim_set_exec_cb(&a, exec_germ_pop);
    lv_anim_set_values(&a, 0, 1000);
    lv_anim_set_duration(&a, 450);
    lv_anim_set_completed_cb(&a, germ_popped_cb);
    lv_anim_start(&a);
    if(sound) PLAY(SND_POP);
}

/* ---------- Character ---------- */

static void set_pose(brushy_pose_t pose)
{
    if(pose == ui.pose) return;
    ui.pose = pose;
    lv_image_set_src(ui.character, ui.ch->poses[pose]);
}

static void set_mood(mood_t mood)
{
    ui.mood = mood;
    ui.mood_since = lv_tick_get();

    lv_anim_delete(ui.character, NULL);
    lv_obj_set_style_translate_y(ui.character, 0, 0);
    lv_image_set_rotation(ui.character, 0);
    lv_image_set_scale(ui.character, 256);

    switch(mood) {
        case MOOD_IDLE:     anim_loop(ui.character, exec_translate_y, 0, -5, 1000, 0); break;
        case MOOD_SLEEPY:   anim_loop(ui.character, exec_breathe, 0, 1000, 1600, 0); break;
        case MOOD_BRUSHING: anim_loop(ui.character, exec_rotation, -20, 20, 560, 0); break;
        case MOOD_CHEER:    anim_loop(ui.character, exec_translate_y, 0, -20, 350, 0); break;
    }
}

/* Picks the frame for the current mood: scrubbing toothbrush, occasional blink. */
static void update_pose(uint32_t now)
{
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

static void giggle(void)
{
    lv_anim_delete(&ui.giggle_token, NULL);
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, &ui.giggle_token);
    lv_anim_set_exec_cb(&a, exec_giggle);
    lv_anim_set_values(&a, 0, 1000);
    lv_anim_set_duration(&a, 500);
    lv_anim_start(&a);
    PLAY(SND_GIGGLE);
}

/* ---------- Overlays and effects ---------- */

static void render_stars(bool bump)
{
    lv_label_set_text_fmt(ui.stars_label, "%" LV_PRIu32, ui.stars);
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
    for(int i = 0; i < CONFETTI; i++) {
        lv_obj_t * c = plain_obj(ui.screen);
        uint32_t color = i % 5 == 4 ? 0xffffff : ui.ch->ring[i % 4];
        lv_obj_set_size(c, 9, 14);
        lv_obj_set_style_radius(c, 3, 0);
        lv_obj_set_style_bg_color(c, lv_color_hex(color), 0);
        lv_obj_set_style_bg_opa(c, LV_OPA_COVER, 0);
        lv_obj_set_pos(c, (int32_t)lv_rand(0, SCREEN_SIZE - 9), -20);

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

static void build_overlays(void)
{
    lv_color_t accent = lv_color_hex(ui.ch->accent);

    /* Star counter (top) */
    ui.stars_row = plain_obj(ui.screen);
    lv_obj_set_size(ui.stars_row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(ui.stars_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ui.stars_row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(ui.stars_row, 6, 0);
    lv_obj_set_style_transform_pivot_x(ui.stars_row, LV_PCT(50), 0);
    lv_obj_set_style_transform_pivot_y(ui.stars_row, LV_PCT(50), 0);
    lv_obj_align(ui.stars_row, LV_ALIGN_TOP_MID, 0, 30);
    lv_obj_t * star = lv_image_create(ui.stars_row);
    lv_image_set_src(star, &img_star);
    ui.stars_label = lv_label_create(ui.stars_row);
    lv_obj_set_style_text_font(ui.stars_label, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(ui.stars_label, lv_color_hex(STAR_COLOR), 0);

    /* Play hint (bottom) */
    ui.hint = plain_obj(ui.screen);
    lv_obj_set_size(ui.hint, 56, 56);
    lv_obj_set_style_radius(ui.hint, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(ui.hint, accent, 0);
    lv_obj_set_style_bg_opa(ui.hint, LV_OPA_COVER, 0);
    lv_obj_set_style_transform_pivot_x(ui.hint, 28, 0);
    lv_obj_set_style_transform_pivot_y(ui.hint, 28, 0);
    lv_obj_align(ui.hint, LV_ALIGN_BOTTOM_MID, 0, -36);
    lv_obj_t * play = lv_label_create(ui.hint);
    lv_label_set_text(play, LV_SYMBOL_PLAY);
    lv_obj_set_style_text_font(play, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(play, lv_color_black(), 0);
    lv_obj_align(play, LV_ALIGN_CENTER, 2, 0);
    anim_loop(ui.hint, exec_scale, 256, 282, 700, 0);

    /* "Bravo!" (bottom, done only) */
    ui.bravo = lv_label_create(ui.screen);
    lv_label_set_text(ui.bravo, "Bravo!");
    lv_obj_set_style_text_font(ui.bravo, &lv_font_montserrat_40, 0);
    lv_obj_set_style_text_color(ui.bravo, accent, 0);
    lv_obj_set_style_transform_pivot_x(ui.bravo, LV_PCT(50), 0);
    lv_obj_set_style_transform_pivot_y(ui.bravo, LV_PCT(50), 0);
    lv_obj_align(ui.bravo, LV_ALIGN_BOTTOM_MID, 0, -52);
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
    ui.phase = phase;
    ui.phase_since = lv_tick_get();
    show(ui.stars_row, phase != PHASE_BRUSHING);
    show(ui.hint, phase == PHASE_IDLE);
    show(ui.bravo, phase == PHASE_DONE);
}

static void to_idle(void)
{
    set_phase(PHASE_IDLE);
    ui.elapsed_ms = 0;
    ui.zone = -1;
    reset_germs();
    set_ring_progress(0);
    set_active_zone(-1);
    set_mood(MOOD_IDLE);
}

static void start(void)
{
    PLAY(SND_START);
    set_phase(PHASE_BRUSHING);
    ui.elapsed_ms = 0;
    ui.zone = -1;
    reset_germs();
    set_mood(MOOD_BRUSHING);
    giggle();
}

static void finish(void)
{
    ui.elapsed_ms = TOTAL_MS;
    for(int i = 0; i < GERM_COUNT; i++) {
        if(!ui.germs[i].popped) pop_germ(&ui.germs[i], false);
    }
    set_ring_progress(TOTAL_MS);
    set_active_zone(-1);
    set_phase(PHASE_DONE);
    set_mood(MOOD_CHEER);
    PLAY(SND_FINISH);
    ui.stars++;
    if(!ui.quiet) brushy_hal_save_stars(ui.stars); /* previews don't earn stars */
    render_stars(true);
    bravo_in();
    confetti();
}

static void update_brushing(void)
{
    float t = ui.elapsed_ms < TOTAL_MS ? ui.elapsed_ms : TOTAL_MS;
    set_ring_progress(t);

    int zone = (int)(t / (TOTAL_MS / ZONES));
    if(zone > ZONES - 1) zone = ZONES - 1;
    if(zone != ui.zone) {
        if(ui.zone != -1) {
            PLAY(SND_ZONE);
            giggle();
        }
        ui.zone = zone;
        set_active_zone(zone);
    }

    for(int i = 0; i < GERM_COUNT; i++) {
        if(!ui.germs[i].popped && t >= ui.germs[i].pop_at_ms) pop_germ(&ui.germs[i], true);
    }

    if(ui.elapsed_ms >= TOTAL_MS) finish();
}

/* ---------- Input and main loop ---------- */

static void on_press(lv_event_t * e)
{
    LV_UNUSED(e);
    if(ui.phase == PHASE_IDLE) {
        start();
    }
    else if(ui.phase == PHASE_BRUSHING) {
        giggle();
    }
    else if(lv_tick_elaps(ui.phase_since) > DONE_TAP_GUARD_MS) {
        to_idle();
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
            if(ui.mood == MOOD_IDLE && in_phase > SLEEP_AFTER_MS) set_mood(MOOD_SLEEPY);
            break;
        case PHASE_DONE:
            if(in_phase > CELEBRATE_MS) to_idle();
            break;
    }
    update_pose(now);
}

/* ---------- Public API ---------- */

void brushy_ui_create(lv_obj_t * screen, const brushy_character_t * character)
{
    memset(&ui, 0, sizeof(ui));
    ui.ch = character;
    ui.screen = screen;
    ui.speed = 1;
    ui.pose = BRUSHY_POSE_COUNT; /* force the first set_pose */
    ui.stars = brushy_hal_load_stars();

    lv_obj_set_style_bg_color(screen, lv_color_black(), 0); /* AMOLED: black = pixels off */
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    lv_obj_remove_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(screen, on_press, LV_EVENT_PRESSED, NULL);

    build_ring();
    build_germs();

    ui.character = lv_image_create(screen);
    lv_obj_set_size(ui.character, CHAR_SIZE, CHAR_SIZE);
    lv_obj_center(ui.character);
    lv_image_set_pivot(ui.character, CHAR_SIZE / 2, CHAR_SIZE); /* bottom centre: feet stay planted */

    build_overlays();
    render_stars(false);
    to_idle();
    update_pose(lv_tick_get());

    ui.last_tick = lv_tick_get();
    lv_timer_create(loop_cb, LOOP_MS, NULL);
}

void brushy_ui_set_speed(float speed)
{
    ui.speed = speed > 0 ? speed : 1;
}

void brushy_ui_preview(const char * state, float seconds)
{
    ui.quiet = true;
    if(strcmp(state, "sleepy") == 0) {
        set_mood(MOOD_SLEEPY);
    }
    else if(strcmp(state, "brushing") == 0) {
        start();
        ui.elapsed_ms = seconds * 1000;
        update_brushing();
    }
    else if(strcmp(state, "done") == 0) {
        start();
        finish();
    }
    update_pose(lv_tick_get());
    ui.quiet = false;
}
