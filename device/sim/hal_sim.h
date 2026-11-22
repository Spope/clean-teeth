#pragma once

#include <stdbool.h>

/** Open the audio device (skipped when `mute` is true). Call after SDL video init. */
void hal_sim_init(bool mute);

/** Sound off or on (the web page's switch). Opens the audio device the first time it's on. */
void hal_sim_set_mute(bool mute);
