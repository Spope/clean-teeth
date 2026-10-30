#include "brushy_theme.h"

#ifdef BRUSHY_THEME
/* Built for one theme, e.g. BRUSHY_THEME=dragon (device/esp32: Kconfig "Brushy Buddy") */
#define THEME_VAR(id)  THEME_VAR_(id)
#define THEME_VAR_(id) brushy_theme_##id
extern const brushy_theme_t THEME_VAR(BRUSHY_THEME);
const brushy_theme_t * const brushy_themes[] = {&THEME_VAR(BRUSHY_THEME)};
#else
extern const brushy_theme_t brushy_theme_minecraft;
extern const brushy_theme_t brushy_theme_dragon;

/* Order in which a long press on the start screen cycles through themes. */
const brushy_theme_t * const brushy_themes[] = {
    &brushy_theme_minecraft,
    &brushy_theme_dragon,
};
#endif
const uint32_t brushy_theme_count = sizeof(brushy_themes) / sizeof(brushy_themes[0]);
