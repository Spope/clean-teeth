/**
 * Effects: germs (wobble, scared shiver, defeat per theme), particle bursts, screen shake,
 * mirror-check sparkles and the end-of-brushing reward drop.
 * Mirrors the matching parts of prototype/js/app.js.
 */
#include "brushy_ui_priv.h"

#include <math.h>

#define GERM_RADIUS  168
#define COUNTER_X    214 /* where the reward flies to: the icon in the counter */
#define COUNTER_Y    46
#define FIREBALL_MS  380
#define PRIME_MS     700 /* creeper swelling before it explodes */

static const brushy_note_t SND_BLIP[] = {{500, 1400, 120, BRUSHY_WAVE_SINE, 80}};
static const brushy_note_t SND_WHOOSH[] = {{600, 2400, 300, BRUSHY_WAVE_NOISE, 90}};
static const brushy_note_t SND_HISS[] = {{3000, 5000, 700, BRUSHY_WAVE_NOISE, 40}}; /* creeper fuse */
static const brushy_note_t SND_BLING[] = {
    {1568, 0, 60, BRUSHY_WAVE_SINE, 40}, {2093, 0, 60, BRUSHY_WAVE_SINE, 40}, {2637, 0, 160, BRUSHY_WAVE_SINE, 40},
};
static const brushy_note_t SND_COLLECT[] = {{988, 0, 60, BRUSHY_WAVE_TRIANGLE, 50}, {1319, 0, 140, BRUSHY_WAVE_TRIANGLE, 50}};

static lv_obj_t * reward_img;
static lv_obj_t * fireball_img;

static void play_pop(void)
{
    if(ui.theme->pop_whoosh) {
        SOUND(SND_WHOOSH);
        return;
    }
    brushy_note_t blip = SND_BLIP[0];
    if(ui.theme->voice == BRUSHY_WAVE_SQUARE) {
        blip.wave = BRUSHY_WAVE_SQUARE;
        blip.volume = 30;
    }
    ui_play(&blip, 1);
}

/* Linear motion in 6 jumps (blocky themes) */
static int32_t path_steps(const lv_anim_t * a)
{
    int32_t step = lv_map(a->act_time, 0, a->duration, 0, 6);
    return lv_map(step > 6 ? 6 : step, 0, 6, a->start_value, a->end_value);
}

static void exec_x(void * obj, int32_t v) { lv_obj_set_x(obj, v); }
static void exec_y(void * obj, int32_t v) { lv_obj_set_y(obj, v); }
static void exec_opa(void * obj, int32_t v) { lv_obj_set_style_opa(obj, (lv_opa_t)v, 0); }
static void exec_translate_x(void * obj, int32_t v) { lv_obj_set_style_translate_x(obj, v, 0); }
static void delete_var_cb(lv_anim_t * a) { lv_obj_delete(a->var); }
static void unhide_cb(lv_anim_t * a) { lv_obj_remove_flag(a->var, LV_OBJ_FLAG_HIDDEN); }

static void anim_once(void * var, lv_anim_exec_xcb_t cb, int32_t from, int32_t to, uint32_t ms, lv_anim_path_cb_t path,
                      lv_anim_completed_cb_t done)
{
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, var);
    lv_anim_set_exec_cb(&a, cb);
    lv_anim_set_values(&a, from, to);
    lv_anim_set_duration(&a, ms);
    lv_anim_set_path_cb(&a, path);
    if(done) lv_anim_set_completed_cb(&a, done);
    lv_anim_start(&a);
}

/* ---------- Particles and shake ---------- */

void fx_burst(int32_t x, int32_t y, const uint32_t * colors, int count, float spread)
{
    bool blocky = ui.theme->style == BRUSHY_STYLE_BLOCKY;
    int ncolors = 0;
    while(ncolors < 4 && colors[ncolors]) ncolors++;
    int32_t size = blocky ? 6 : 7;
    lv_anim_path_cb_t path = blocky ? path_steps : lv_anim_path_ease_out;

    for(int i = 0; i < count; i++) {
        lv_obj_t * p = ui_plain_obj(ui.stage);
        lv_obj_set_size(p, size, size);
        lv_obj_set_style_radius(p, blocky ? 0 : LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(p, lv_color_hex(colors[i % ncolors]), 0);
        lv_obj_set_style_bg_opa(p, LV_OPA_COVER, 0);
        int32_t x0 = x - size / 2, y0 = y - size / 2;
        lv_obj_set_pos(p, x0, y0);

        float a = (float)i / count * 6.2831853f + (float)lv_rand(0, 60) / 100.0f - 0.3f;
        float d = (float)lv_rand(35, 70) * spread;
        uint32_t ms = lv_rand(450, 650);
        anim_once(p, exec_x, x0, x0 + (int32_t)(cosf(a) * d), ms, path, NULL);
        anim_once(p, exec_y, y0, y0 + (int32_t)(sinf(a) * d) + 25, ms, path, NULL);
        anim_once(p, exec_opa, LV_OPA_COVER, LV_OPA_TRANSP, ms, lv_anim_path_linear, delete_var_cb);
    }
}

/* v: 0..1000 -> quick damped wobble (same curve as the prototype's shake) */
static void exec_shake(void * obj, int32_t v)
{
    float p = v / 1000.0f;
    float x = 7.0f * (1 - p) * sinf(p * 3.14159265f * 8);
    float y = 3.5f * (1 - p) * sinf(p * 3.14159265f * 6);
    lv_obj_set_pos(obj, (int32_t)lroundf(x), (int32_t)lroundf(y));
}

void fx_shake(void)
{
    lv_anim_delete(ui.stage, exec_shake);
    anim_once(ui.stage, exec_shake, 0, 1000, 350, lv_anim_path_linear, NULL);
}

/* ---------- Germs ---------- */

static void germ_wobble(germ_t * g)
{
    ui_anim_loop(g->img, ui_exec_rotation, -100, 100, 1400, (uint32_t)((g - ui.germs) * 230 % 1400));
}

void fx_build_germs(void)
{
    const float zone_ms = (float)TOTAL_MS / ZONES;
    for(int z = 0; z < ZONES; z++) {
        for(int i = 0; i < GERMS_PER_ZONE; i++) {
            float deg = 180.0f + z * 90.0f + 90.0f * (i + 1) / (GERMS_PER_ZONE + 1);
            float rad = deg * 3.14159265f / 180.0f;
            germ_t * g = &ui.germs[z * GERMS_PER_ZONE + i];
            g->kind = (uint8_t)((z + i) % 4);
            g->cx = (int32_t)(CENTER + GERM_RADIUS * cosf(rad));
            g->cy = (int32_t)(CENTER + GERM_RADIUS * sinf(rad));
            g->img = ui_image(ui.stage, ui.theme->germs[g->kind]);
            lv_image_set_pivot(g->img, GERM_SIZE / 2, GERM_SIZE / 2);
            lv_obj_set_pos(g->img, g->cx - GERM_SIZE / 2, g->cy - GERM_SIZE / 2);
            lv_obj_set_style_image_recolor(g->img, lv_color_white(), 0); /* for the creeper flash */
            g->pop_at_ms = (uint32_t)(z * zone_ms + (i + 1) * zone_ms / GERMS_PER_ZONE);
        }
    }
}

static void calm_germ(germ_t * g)
{
    lv_anim_delete(g->img, NULL);
    lv_image_set_src(g->img, ui.theme->germs[g->kind]);
    lv_obj_set_style_translate_x(g->img, 0, 0);
    lv_obj_set_style_image_recolor_opa(g->img, LV_OPA_TRANSP, 0);
    lv_image_set_scale(g->img, 256);
    lv_image_set_rotation(g->img, 0);
}

void fx_reset_germs(void)
{
    for(int i = 0; i < GERM_COUNT; i++) {
        germ_t * g = &ui.germs[i];
        g->popped = false;
        calm_germ(g);
        lv_obj_remove_flag(g->img, LV_OBJ_FLAG_HIDDEN);
        germ_wobble(g);
    }
}

void fx_pop_germ_quietly(germ_t * g)
{
    g->popped = true;
    lv_anim_delete(g->img, NULL);
    lv_obj_add_flag(g->img, LV_OBJ_FLAG_HIDDEN);
}

/* v: 0..1000 -> grow to 1.5x, then shrink to nothing */
static void exec_germ_pop(void * obj, int32_t v)
{
    int32_t s = v < 400 ? 256 + 128 * v / 400 : 384 * (1000 - v) / 600;
    lv_image_set_scale(obj, (uint32_t)LV_MAX(s, 1));
}

static void germ_hidden_cb(lv_anim_t * a) { lv_obj_add_flag(a->var, LV_OBJ_FLAG_HIDDEN); }

/* When one germ pops, the others shiver with a scared face for a moment. */
static void scared_done_cb(lv_anim_t * a)
{
    germ_t * g = lv_anim_get_user_data(a);
    if(g->popped) return;
    calm_germ(g);
    germ_wobble(g);
}

static void scare_germs(void)
{
    for(int i = 0; i < GERM_COUNT; i++) {
        germ_t * g = &ui.germs[i];
        if(g->popped) continue;
        calm_germ(g);
        lv_image_set_src(g->img, ui.theme->germs_scared[g->kind]);
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, g->img);
        lv_anim_set_exec_cb(&a, exec_translate_x);
        lv_anim_set_values(&a, -3, 3);
        lv_anim_set_duration(&a, 70);
        lv_anim_set_reverse_duration(&a, 70);
        lv_anim_set_repeat_count(&a, 6);
        lv_anim_set_user_data(&a, g);
        lv_anim_set_completed_cb(&a, scared_done_cb);
        lv_anim_start(&a);
    }
}

static void explode(germ_t * g)
{
    g->popped = true;
    calm_germ(g);
    anim_once(g->img, exec_germ_pop, 0, 1000, 450, lv_anim_path_linear, germ_hidden_cb);
    fx_burst(g->cx, g->cy, ui.theme->germ_particles[g->kind], 12, 1);
    play_pop();
    fx_shake();
    scare_germs();
}

/* Creeper: swells in steps while flashing white, then explodes */
static void exec_prime(void * obj, int32_t v)
{
    int32_t grow = v >= 600 ? 90 : v >= 420 ? 50 : v >= 210 ? 25 : 0;
    lv_image_set_scale(obj, (uint32_t)(256 + grow));
    lv_obj_set_style_image_recolor_opa(obj, (v / 105) % 2 ? 210 : 0, 0);
}

static void primed_cb(lv_anim_t * a)
{
    germ_t * g = lv_anim_get_user_data(a);
    if(ui.phase == PHASE_BRUSHING) explode(g);
}

static void prime(germ_t * g)
{
    lv_anim_delete(g->img, NULL);
    lv_image_set_rotation(g->img, 0);
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, g->img);
    lv_anim_set_exec_cb(&a, exec_prime);
    lv_anim_set_values(&a, 0, PRIME_MS);
    lv_anim_set_duration(&a, PRIME_MS);
    lv_anim_set_user_data(&a, g);
    lv_anim_set_completed_cb(&a, primed_cb);
    lv_anim_start(&a);
    SOUND(SND_HISS);
}

/* Fireball from the buddy's mouth to the germ */
static void exec_fireball(lv_anim_t * a, int32_t v)
{
    germ_t * g = lv_anim_get_user_data(a);
    float p = ui_ease(EASE_IN, v / 1000.0f);
    int32_t x0 = BUDDY_X + ui.theme->mouth_x, y0 = BUDDY_Y + ui.theme->mouth_y;
    int32_t w = ui.theme->fireball->header.w, h = ui.theme->fireball->header.h;
    lv_obj_set_pos(a->var, x0 + (int32_t)((g->cx - x0) * p) - w / 2, y0 + (int32_t)((g->cy - y0) * p) - h / 2);
    lv_image_set_scale(a->var, (uint32_t)(102 + 180 * p)); /* 0.4x -> 1.1x */
}

static void fireball_done_cb(lv_anim_t * a)
{
    germ_t * g = lv_anim_get_user_data(a);
    lv_obj_delete(a->var);
    fireball_img = NULL;
    if(ui.phase == PHASE_BRUSHING) explode(g);
}

static void breathe_fire(germ_t * g)
{
    tricks_stop();
    if(ui.pose_override == BRUSHY_POSE_COUNT) ui_override_pose(BRUSHY_POSE_FIRE, FIREBALL_MS + 100);
    if(fireball_img) lv_obj_delete(fireball_img);
    fireball_img = ui_image(ui.stage, ui.theme->fireball);
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, fireball_img);
    lv_anim_set_custom_exec_cb(&a, exec_fireball);
    lv_anim_set_values(&a, 0, 1000);
    lv_anim_set_duration(&a, FIREBALL_MS);
    lv_anim_set_user_data(&a, g);
    lv_anim_set_completed_cb(&a, fireball_done_cb);
    lv_anim_start(&a);
}

void fx_defeat_germ(germ_t * g)
{
    g->popped = true; /* claimed: won't be triggered again */
    if(ui.theme->pop == BRUSHY_POP_CREEPER && g->kind == 0) prime(g);
    else if(ui.theme->pop == BRUSHY_POP_FIREBALL && ui.theme->fireball) breathe_fire(g);
    else explode(g);
}

/* ---------- Mirror check: proud grin with sparkles on the teeth ---------- */

/* v: 0..1000 -> grow to 1.2x and back while turning half a turn */
static void exec_sparkle(void * obj, int32_t v)
{
    int32_t s = v < 500 ? v * 307 / 500 : (1000 - v) * 307 / 500;
    lv_image_set_scale(obj, (uint32_t)LV_MAX(s, 1));
    lv_image_set_rotation(obj, v * 1800 / 1000);
}

static void sparkle(int32_t x, int32_t y, uint32_t delay)
{
    const lv_image_dsc_t * src = ui.theme->sparkle;
    lv_obj_t * s = ui_image(ui.stage, src);
    lv_obj_set_pos(s, x - src->header.w / 2, y - src->header.h / 2);
    lv_image_set_pivot(s, src->header.w / 2, src->header.h / 2);
    lv_obj_add_flag(s, LV_OBJ_FLAG_HIDDEN);
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, s);
    lv_anim_set_exec_cb(&a, exec_sparkle);
    lv_anim_set_values(&a, 0, 1000);
    lv_anim_set_duration(&a, 800);
    lv_anim_set_delay(&a, delay);
    lv_anim_set_start_cb(&a, unhide_cb);
    lv_anim_set_completed_cb(&a, delete_var_cb);
    lv_anim_start(&a);
}

void fx_mirror_check(void)
{
    ui.mirrored = true;
    tricks_stop();
    ui_override_pose(BRUSHY_POSE_GRIN, 1700);
    SOUND(SND_BLING);
    int32_t x = BUDDY_X + ui.theme->teeth_x, y = BUDDY_Y + ui.theme->teeth_y;
    sparkle(x - 22, y - 6, 0);
    sparkle(x + 24, y + 4, 300);
    sparkle(x - 4, y - 14, 700);
}

/* ---------- Reward drop: pops out of the buddy, bounces, flies to the counter ---------- */

static keyframe_t reward_path[7];

static void exec_reward(void * obj, int32_t t)
{
    int32_t v[4];
    ui_keyframes(reward_path, 7, t, ui.theme->style == BRUSHY_STYLE_BLOCKY ? 4 : 0, v); /* pixel steps */
    lv_obj_set_pos(obj, v[0] - 16, v[1] - 16);
    lv_image_set_scale(obj, (uint32_t)v[2]);
}

static void reward_landed_cb(lv_anim_t * a)
{
    lv_obj_delete(a->var);
    reward_img = NULL;
    if(ui.phase != PHASE_DONE) return;
    ui_render_stars(true, ui.stars);
    MELODY(SND_COLLECT);
}

void fx_reward_drop(void)
{
    int d = lv_rand(0, 1) ? 1 : -1;
    reward_path[0] = FRAME(0, CENTER, 260, 0.3f, 0, EASE_OUT);
    reward_path[1] = FRAME(300, CENTER + d * 30, 165, 1.6f, 0, EASE_IN);
    reward_path[2] = FRAME(650, CENTER + d * 55, 330, 1.6f, 0, EASE_OUT);
    reward_path[3] = FRAME(800, CENTER + d * 60, 305, 1.6f, 0, EASE_IN);
    reward_path[4] = FRAME(950, CENTER + d * 62, 330, 1.6f, 0, EASE_LINEAR);
    reward_path[5] = FRAME(1500, CENTER + d * 62, 330, 1.6f, 0, EASE_IN);
    reward_path[6] = FRAME(1900, COUNTER_X, COUNTER_Y, 1, 0, EASE_LINEAR);

    if(reward_img) lv_obj_delete(reward_img);
    reward_img = ui_image(ui.stage, ui.theme->reward);
    lv_image_set_pivot(reward_img, 16, 16);
    exec_reward(reward_img, 0);
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, reward_img);
    lv_anim_set_exec_cb(&a, exec_reward);
    lv_anim_set_values(&a, 0, 1900);
    lv_anim_set_duration(&a, 1900);
    lv_anim_set_completed_cb(&a, reward_landed_cb);
    lv_anim_start(&a);
}

void fx_clear(void)
{
    if(reward_img) lv_obj_delete(reward_img);
    if(fireball_img) lv_obj_delete(fireball_img);
    reward_img = fireball_img = NULL;
}
