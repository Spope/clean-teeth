/**
 * What the web page (web/index.html) calls, in the WebAssembly build only: the device UI's
 * API (brushy_ui.h) for its control panel, the themes built in, and the sound switch.
 */
#include <emscripten.h>

#include "brushy_ui.h"
#include "hal_sim.h"

EMSCRIPTEN_KEEPALIVE int web_theme_count(void) { return (int)brushy_theme_count; }
EMSCRIPTEN_KEEPALIVE const char * web_theme_id(int i) { return brushy_themes[i]->id; }
EMSCRIPTEN_KEEPALIVE const char * web_theme_name(int i) { return brushy_themes[i]->name; }
EMSCRIPTEN_KEEPALIVE uint32_t web_theme_accent(int i) { return brushy_themes[i]->accent; }
EMSCRIPTEN_KEEPALIVE const char * web_theme(void) { return brushy_ui_theme()->id; }
EMSCRIPTEN_KEEPALIVE bool web_set_theme(const char * id) { return brushy_ui_set_theme(id); }

EMSCRIPTEN_KEEPALIVE bool web_trick(const char * name) { return brushy_ui_trick(name); }
EMSCRIPTEN_KEEPALIVE bool web_has_trick(const char * name) { return brushy_ui_has_trick(name); }

EMSCRIPTEN_KEEPALIVE void web_set_speed(float speed) { brushy_ui_set_speed(speed); }
EMSCRIPTEN_KEEPALIVE void web_preview(const char * state, float seconds) { brushy_ui_preview(state, seconds); }
EMSCRIPTEN_KEEPALIVE const char * web_state(void) { return brushy_ui_state(); }
EMSCRIPTEN_KEEPALIVE float web_elapsed(void) { return brushy_ui_elapsed(); }

EMSCRIPTEN_KEEPALIVE void web_battery(int percent) { brushy_ui_battery(percent); }
EMSCRIPTEN_KEEPALIVE void web_charging(int percent) { brushy_ui_charging(percent); }
EMSCRIPTEN_KEEPALIVE void web_set_mute(bool mute) { hal_sim_set_mute(mute); }
