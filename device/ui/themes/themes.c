#include "brushy_theme.h"

extern const brushy_theme_t brushy_theme_minecraft;
extern const brushy_theme_t brushy_theme_dragon;

/* Order in which a long press on the start screen cycles through themes. */
const brushy_theme_t * const brushy_themes[] = {
    &brushy_theme_minecraft,
    &brushy_theme_dragon,
};
const uint32_t brushy_theme_count = sizeof(brushy_themes) / sizeof(brushy_themes[0]);
