/**
 * Buddy tricks: keyframed movement around the buddy's feet, plus timed events
 * (sounds, the mining block) and poses. Keyframes mirror TRICKS in prototype/js/app.js.
 *   peek       duck out, pop up big and close from the bottom edge, duck, come back
 *   peek_side  slip off one side, lean in from that edge to spy (twice), walk back
 *   slide      walk off one side, hop across the whole screen, walk back in
 *   mine       Minecraft: hit a grass block three times until it breaks
 *   rainbow    dragon: scales cycle through the rainbow while hopping
 *   fly        dragon: take off, fly across the top of the screen, glide back
 * Generic tricks play on the start screen and during brushing; theme tricks only on the
 * start screen (they put the toothbrush away).
 */
#include "brushy_ui_priv.h"

#define TRICK_EVERY_MIN 8000
#define TRICK_EVERY_MAX 14000
#define MAX_FRAMES      16
#define MAX_EVENTS      12
#define OFF             420 /* fully outside the 466 px screen */
#define HIT_MS          180 /* how long the pickaxe stays down */
#define RAINBOW_STEP_MS 320

typedef enum { EV_BLOCK, EV_HIT, EV_BREAK, EV_MAGIC, EV_WHOOSH } event_t;

typedef struct {
    uint16_t ms;
    uint8_t code;
} trick_event_t;

static struct {
    bool active;
    trick_t kind;
    keyframe_t frames[MAX_FRAMES];
    uint8_t n_frames;
    trick_event_t events[MAX_EVENTS];
    uint8_t n_events;
    uint8_t next_event;
    uint32_t next_at;
    lv_obj_t * block;
    uint8_t hits;
    int token; /* address used as the animation's var */
} tr;

static const uint16_t MINE_HITS[] = {500, 1100, 1700};
static const uint32_t DIRT[4] = {0x8b5a2b, 0x6e4520, 0x5dbb3a, 0};

static const brushy_note_t SND_HIT[] = {{160, 90, 60, BRUSHY_WAVE_SQUARE, 50}};
static const brushy_note_t SND_CRUMBLE[] = {{400, 150, 250, BRUSHY_WAVE_NOISE, 90}};
static const brushy_note_t SND_WHOOSH[] = {{600, 2400, 300, BRUSHY_WAVE_NOISE, 90}};
static const brushy_note_t SND_MAGIC[] = {
    {784, 0, 70, BRUSHY_WAVE_SINE, 35}, {988, 0, 70, BRUSHY_WAVE_SINE, 35}, {1175, 0, 70, BRUSHY_WAVE_SINE, 35},
    {1568, 0, 70, BRUSHY_WAVE_SINE, 35}, {1976, 0, 160, BRUSHY_WAVE_SINE, 35},
};

/* ---------- Keyframes ---------- */

static void add_frame(keyframe_t f) { if(tr.n_frames < MAX_FRAMES) tr.frames[tr.n_frames++] = f; }
static void add_event(uint16_t ms, uint8_t code) { if(tr.n_events < MAX_EVENTS) tr.events[tr.n_events++] = (trick_event_t){ms, code}; }

/* side: -1 left, 0 middle, 1 right (leaning in) */
static void build_peek(int side)
{
    int x = side * 90, r = -side * 15;
    add_frame(FRAME(0, 0, 0, 1, 0, EASE_IN));
    add_frame(FRAME(350, 0, 400, 1, 0, EASE_LINEAR));
    add_frame(FRAME(360, x, 700, 1.9f, r, EASE_BACK));
    add_frame(FRAME(850, x, 287, 1.9f, r, EASE_LINEAR));
    add_frame(FRAME(2000, x, 287, 1.9f, r, EASE_IN));
    add_frame(FRAME(2400, x, 700, 1.9f, r, EASE_LINEAR));
    add_frame(FRAME(2410, 0, 400, 1, 0, EASE_BACK));
    add_frame(FRAME(2900, 0, 0, 1, 0, EASE_LINEAR));
}

static void build_peek_side(int d)
{
    int x = d * 255, r = -d * 22;
    add_frame(FRAME(0, 0, 0, 1, 0, EASE_IN));
    add_frame(FRAME(400, d * OFF, 0, 1, 0, EASE_LINEAR));
    add_frame(FRAME(410, d * OFF, 20, 1, r, EASE_OUT));
    add_frame(FRAME(900, x, 20, 1, r, EASE_LINEAR));
    add_frame(FRAME(1800, x, 20, 1, r, EASE_IN));
    add_frame(FRAME(2100, d * OFF, 20, 1, r, EASE_LINEAR));
    add_frame(FRAME(2500, d * OFF, 20, 1, r, EASE_OUT));
    add_frame(FRAME(2800, x, 20, 1, r, EASE_LINEAR));
    add_frame(FRAME(3500, x, 20, 1, r, EASE_IN));
    add_frame(FRAME(3800, d * OFF, 20, 1, r, EASE_LINEAR));
    add_frame(FRAME(3810, d * OFF, 0, 1, 0, EASE_OUT));
    add_frame(FRAME(4400, 0, 0, 1, 0, EASE_LINEAR));
}

/* Off one side, across in 8 hops (y: hop height, rot: lean), back in from the other side */
static void build_crossing(int d, int start_ms, int hop_ms, int y, int hop, int lean)
{
    for(int k = 0; k <= 8; k++) {
        add_frame(FRAME(start_ms + k * hop_ms, d * (OFF - k * 2 * OFF / 8), y + (k % 2 ? hop : 0), 1, -d * lean, EASE_LINEAR));
    }
}

static void build_slide(int d)
{
    add_frame(FRAME(0, 0, 0, 1, 0, EASE_IN));
    add_frame(FRAME(150, d * 15, -6, 1, d * 6, EASE_IN));
    add_frame(FRAME(700, d * OFF, 0, 1, d * 6, EASE_LINEAR));
    build_crossing(d, 1100, 250, 0, -10, 6);
    add_frame(FRAME(3500, -d * OFF, 0, 1, d * 6, EASE_OUT));
    add_frame(FRAME(4200, -d * 15, -6, 1, d * 6, EASE_OUT));
    add_frame(FRAME(4400, 0, 0, 1, 0, EASE_LINEAR));
}

static void build_mine(void)
{
    add_frame(FRAME(0, 0, 0, 1, 0, EASE_LINEAR));
    add_frame(FRAME(3000, 0, 0, 1, 0, EASE_LINEAR));
    add_event(0, EV_BLOCK);
    add_event(MINE_HITS[0], EV_HIT);
    add_event(MINE_HITS[1], EV_HIT);
    add_event(MINE_HITS[2], EV_BREAK);
}

static void build_rainbow(void)
{
    for(int k = 0; k <= 8; k++) add_frame(FRAME(k * 400, 0, k % 2 ? -12 : 0, 1, 0, k % 2 ? EASE_IN : EASE_OUT));
    add_event(0, EV_MAGIC);
}

static void build_fly(int d)
{
    add_frame(FRAME(0, 0, 0, 1, 0, EASE_IN));
    add_frame(FRAME(400, d * 40, -50, 1, d * 8, EASE_IN));
    add_frame(FRAME(900, d * OFF, -120, 1, d * 10, EASE_LINEAR));
    build_crossing(d, 1300, 225, -110, -25, 10);
    add_frame(FRAME(3500, -d * OFF, -120, 1, d * 10, EASE_OUT));
    add_frame(FRAME(4300, -d * 40, -40, 1, d * 6, EASE_OUT));
    add_frame(FRAME(4700, 0, 0, 1, 0, EASE_LINEAR));
    add_event(0, EV_WHOOSH);
    add_event(3500, EV_WHOOSH);
}

/* ---------- Running a trick ---------- */

static void place_buddy(int32_t x, int32_t y, uint32_t scale, int32_t rot)
{
    lv_obj_align(ui.character, LV_ALIGN_CENTER, x, y);
    lv_image_set_scale(ui.character, scale);
    lv_image_set_rotation(ui.character, rot);
}

static void exec_scale(void * obj, int32_t v) { lv_image_set_scale(obj, (uint32_t)v); }

static void handle_event(uint8_t code)
{
    int32_t bx = BUDDY_X + 224, by = BUDDY_Y + 226; /* block centre, by Steve's right foot */
    switch(code) {
        case EV_BLOCK: {
            tr.block = ui_image(ui.stage, ui.theme->block);
            lv_obj_move_to_index(tr.block, lv_obj_get_index(ui.character)); /* behind the buddy */
            lv_obj_set_pos(tr.block, bx - 24, by - 24);
            lv_image_set_pivot(tr.block, 24, 24);
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, tr.block);
            lv_anim_set_exec_cb(&a, exec_scale);
            lv_anim_set_values(&a, 64, 256);
            lv_anim_set_duration(&a, 200);
            lv_anim_start(&a);
            break;
        }
        case EV_HIT:
            SOUND(SND_HIT);
            fx_burst(bx, by - 20, DIRT, 4, 0.5f);
            if(++tr.hits == 2 && tr.block) lv_image_set_src(tr.block, ui.theme->block_cracked);
            break;
        case EV_BREAK:
            SOUND(SND_HIT);
            SOUND(SND_CRUMBLE);
            if(tr.block) lv_obj_delete(tr.block);
            tr.block = NULL;
            fx_burst(bx, by, DIRT, 16, 1);
            fx_shake();
            break;
        case EV_MAGIC: SOUND(SND_MAGIC); break;
        case EV_WHOOSH: SOUND(SND_WHOOSH); break;
    }
}

/* The pose some tricks need at time t (others keep the mood's own frames). */
static void trick_pose(int32_t t)
{
    if(tr.kind == TRICK_MINE) {
        bool hit = false;
        for(unsigned i = 0; i < sizeof(MINE_HITS) / sizeof(MINE_HITS[0]); i++) {
            if(t >= MINE_HITS[i] && t < MINE_HITS[i] + HIT_MS) hit = true;
        }
        ui_override_pose(hit ? BRUSHY_POSE_MINE_B : BRUSHY_POSE_MINE_A, 0);
    }
    else if(tr.kind == TRICK_RAINBOW) {
        if(t < RAINBOW_STEP_MS * 10) ui_override_pose((brushy_pose_t)(BRUSHY_POSE_RAINBOW_1 + (t / RAINBOW_STEP_MS) % 5), 0);
        else ui_clear_pose_override();
    }
}

static void exec_trick(void * var, int32_t t)
{
    LV_UNUSED(var);
    int32_t v[4];
    ui_keyframes(tr.frames, tr.n_frames, t, 0, v);
    place_buddy(v[0], v[1], (uint32_t)v[2], v[3]);
    trick_pose(t);
    while(tr.next_event < tr.n_events && tr.events[tr.next_event].ms <= t) handle_event(tr.events[tr.next_event++].code);
}

static void cleanup(void)
{
    tr.active = false;
    if(tr.block) lv_obj_delete(tr.block);
    tr.block = NULL;
    ui_clear_pose_override();
    place_buddy(0, 0, 256, 0);
}

static void trick_done_cb(lv_anim_t * a)
{
    LV_UNUSED(a);
    cleanup();
    ui_start_mood_motion(ui.mood);
    tricks_schedule();
}

static bool available(trick_t kind)
{
    const brushy_theme_t * t = ui.theme;
    switch(kind) {
        case TRICK_MINE: return (t->tricks & BRUSHY_TRICK_MINE) && t->block && t->poses[BRUSHY_POSE_MINE_A];
        case TRICK_RAINBOW: return (t->tricks & BRUSHY_TRICK_RAINBOW) && t->poses[BRUSHY_POSE_RAINBOW_5];
        case TRICK_FLY: return (t->tricks & BRUSHY_TRICK_FLY) && t->poses[BRUSHY_POSE_FLY_A];
        default: return true;
    }
}

bool tricks_start(trick_t kind)
{
    if(!available(kind)) return false;
    tricks_stop();
    tr.kind = kind;
    tr.n_frames = tr.n_events = tr.next_event = tr.hits = 0;
    int side = lv_rand(0, 1) ? 1 : -1;
    switch(kind) {
        case TRICK_PEEK: build_peek((int)lv_rand(0, 2) - 1); break;
        case TRICK_PEEK_SIDE: build_peek_side(side); break;
        case TRICK_SLIDE: build_slide(side); break;
        case TRICK_MINE: build_mine(); break;
        case TRICK_RAINBOW: build_rainbow(); break;
        case TRICK_FLY: build_fly(side); break;
        default: return false;
    }

    lv_anim_delete(ui.character, NULL); /* the trick owns the buddy's movement meanwhile */
    lv_obj_set_style_translate_y(ui.character, 0, 0);
    ui_clear_pose_override();
    ui.flying = kind == TRICK_FLY;
    tr.active = true;

    uint32_t total = tr.frames[tr.n_frames - 1].ms;
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, &tr.token);
    lv_anim_set_exec_cb(&a, exec_trick);
    lv_anim_set_values(&a, 0, (int32_t)total);
    lv_anim_set_duration(&a, total);
    lv_anim_set_completed_cb(&a, trick_done_cb);
    lv_anim_start(&a);
    exec_trick(NULL, 0);
    return true;
}

void tricks_stop(void)
{
    if(!tr.active) return;
    lv_anim_delete(&tr.token, NULL);
    cleanup();
    ui_start_mood_motion(ui.mood);
}

bool tricks_active(void)
{
    return tr.active;
}

void tricks_schedule(void)
{
    tr.next_at = lv_tick_get() + lv_rand(TRICK_EVERY_MIN, TRICK_EVERY_MAX);
}

void tricks_update(uint32_t now)
{
    if(tr.active || (int32_t)(now - tr.next_at) < 0) return;
    bool idle = ui.phase == PHASE_IDLE && ui.mood == MOOD_IDLE;
    if(!(idle || ui.phase == PHASE_BRUSHING) || ui.pose_override != BRUSHY_POSE_COUNT) return;

    trick_t pool[TRICK_COUNT];
    int n = 0;
    pool[n++] = TRICK_PEEK;
    pool[n++] = TRICK_PEEK_SIDE;
    pool[n++] = TRICK_SLIDE;
    if(idle) { /* theme tricks put the toothbrush away */
        for(trick_t k = TRICK_MINE; k < TRICK_COUNT; k++) {
            if(available(k)) pool[n++] = k;
        }
    }
    tricks_start(pool[lv_rand(0, n - 1)]);
}
