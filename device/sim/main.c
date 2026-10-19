/**
 * Desktop simulator: runs the device UI in a 466x466 SDL window.
 *
 *   brushy_sim [--theme ID] [--speed N] [--preview STATE[:SECONDS]] [--trick NAME] [--mute]
 *              [--zoom F] [--snapshot FILE.bmp [--after MS]]
 *
 * ID is minecraft | dragon (default: the last one used; long-press to switch).
 * STATE is idle | sleepy | brushing | done (e.g. --preview brushing:50).
 * NAME is peek | peek_side | slide | mine | rainbow | fly | mirror | shake: play it right away.
 * --snapshot renders the screen to a BMP after MS milliseconds and exits.
 */
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lvgl.h"
#include "brushy_ui.h"
#include "hal_sim.h"

#define SCREEN_SIZE 466
#define BEZEL_COLOR 0x1b1b20

/* The real panel is round: hide the corners of the square window. */
static void add_round_mask(void)
{
    const int32_t border = 110; /* reaches past the window corners */
    lv_obj_t * layer = lv_layer_top();
    lv_obj_remove_flag(layer, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * mask = lv_obj_create(layer);
    lv_obj_remove_style_all(mask);
    lv_obj_remove_flag(mask, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(mask, SCREEN_SIZE + 2 * border, SCREEN_SIZE + 2 * border);
    lv_obj_set_pos(mask, -border, -border);
    lv_obj_set_style_radius(mask, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(mask, border, 0);
    lv_obj_set_style_border_color(mask, lv_color_hex(BEZEL_COLOR), 0);
    lv_obj_set_style_border_opa(mask, LV_OPA_COVER, 0);
}

/* 24-bit BMP of the active screen, corners filled with the bezel colour. */
static bool write_snapshot(const char * path)
{
    lv_draw_buf_t * snap = lv_snapshot_take(lv_screen_active(), LV_COLOR_FORMAT_RGB888);
    if(!snap) return false;

    FILE * f = fopen(path, "wb");
    if(!f) {
        lv_draw_buf_destroy(snap);
        return false;
    }

    const uint32_t w = snap->header.w, h = snap->header.h;
    const uint32_t row = (w * 3 + 3) & ~3u;
    const uint32_t size = 54 + row * h;
    uint8_t hdr[54] = {'B', 'M'};
    memcpy(hdr + 2, &size, 4);
    hdr[10] = 54;
    hdr[14] = 40;
    memcpy(hdr + 18, &w, 4);
    memcpy(hdr + 22, &h, 4);
    hdr[26] = 1;
    hdr[28] = 24;
    fwrite(hdr, 1, sizeof(hdr), f);

    uint8_t * line = calloc(row, 1);
    const float r = w / 2.0f;
    for(int32_t y = (int32_t)h - 1; y >= 0; y--) { /* BMP rows go bottom-up */
        const uint8_t * src = snap->data + (uint32_t)y * snap->header.stride;
        for(uint32_t x = 0; x < w; x++) {
            float dx = x + 0.5f - r, dy = y + 0.5f - r;
            if(dx * dx + dy * dy > r * r) { /* outside the round panel */
                line[x * 3 + 0] = BEZEL_COLOR & 0xff;
                line[x * 3 + 1] = (BEZEL_COLOR >> 8) & 0xff;
                line[x * 3 + 2] = (BEZEL_COLOR >> 16) & 0xff;
            }
            else { /* LVGL's RGB888 is stored B,G,R like BMP */
                memcpy(&line[x * 3], &src[x * 3], 3);
            }
        }
        fwrite(line, 1, row, f);
    }
    free(line);
    fclose(f);
    lv_draw_buf_destroy(snap);
    return true;
}

int main(int argc, char ** argv)
{
    float speed = 1, zoom = 1, preview_seconds = 0;
    const char * preview = NULL;
    const char * theme = NULL;
    const char * trick = NULL;
    const char * snapshot = NULL;
    uint32_t snapshot_after = 1500;
    bool mute = false;
    static char preview_buf[32];

    for(int i = 1; i < argc; i++) {
        const char * a = argv[i];
        const char * next = i + 1 < argc ? argv[i + 1] : NULL;
        if(!strcmp(a, "--speed") && next) speed = strtof(argv[++i], NULL);
        else if(!strcmp(a, "--zoom") && next) zoom = strtof(argv[++i], NULL);
        else if(!strcmp(a, "--after") && next) snapshot_after = (uint32_t)strtoul(argv[++i], NULL, 10);
        else if(!strcmp(a, "--snapshot") && next) snapshot = argv[++i];
        else if(!strcmp(a, "--theme") && next) theme = argv[++i];
        else if(!strcmp(a, "--trick") && next) trick = argv[++i];
        else if(!strcmp(a, "--mute")) mute = true;
        else if(!strcmp(a, "--preview") && next) {
            snprintf(preview_buf, sizeof(preview_buf), "%s", argv[++i]);
            char * colon = strchr(preview_buf, ':');
            if(colon) {
                *colon = '\0';
                preview_seconds = strtof(colon + 1, NULL);
            }
            preview = preview_buf;
        }
        else {
            fprintf(stderr,
                    "usage: %s [--theme ID] [--speed N] [--preview STATE[:SECONDS]] [--trick NAME]\n"
                    "          [--mute] [--zoom F]\n"
                    "          [--snapshot FILE.bmp [--after MS]]\n", argv[0]);
            return 1;
        }
    }

    lv_init();
    lv_tick_set_cb(SDL_GetTicks);
    lv_display_t * disp = lv_sdl_window_create(SCREEN_SIZE, SCREEN_SIZE);
    lv_sdl_window_set_title(disp, "Brushy Buddy (simulator)");
    if(zoom != 1) lv_sdl_window_set_zoom(disp, zoom);
    lv_sdl_mouse_create();
    hal_sim_init(mute || snapshot);

    brushy_ui_create(lv_screen_active());
    if(theme && !brushy_ui_set_theme(theme)) {
        fprintf(stderr, "unknown theme '%s'\n", theme);
        return 1;
    }
    brushy_ui_set_speed(speed);
    if(preview) brushy_ui_preview(preview, preview_seconds);
    if(trick && !brushy_ui_trick(trick)) {
        fprintf(stderr, "unknown trick '%s' (or not in this theme)\n", trick);
        return 1;
    }
    add_round_mask();

    const uint32_t started = SDL_GetTicks();
    while(1) {
        uint32_t idle = lv_timer_handler();
        if(snapshot && SDL_GetTicks() - started >= snapshot_after) {
            bool ok = write_snapshot(snapshot);
            fprintf(ok ? stdout : stderr, ok ? "snapshot written to %s\n" : "snapshot failed: %s\n", snapshot);
            return ok ? 0 : 1;
        }
        SDL_Delay(idle < 5 ? idle : 5);
    }
}
