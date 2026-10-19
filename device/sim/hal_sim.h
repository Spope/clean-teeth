#pragma once

#include <stdbool.h>

/** Open the audio device (skipped when `mute` is true). Call after SDL video init. */
void hal_sim_init(bool mute);
