/**
 * Tiny synthesizer for brushy_note_t sequences, shared by the platform layers
 * (simulator and device) so both sound the same.
 */
#pragma once

#include <stdint.h>
#include "brushy_hal.h"

/** Number of samples brushy_synth_render() writes for these notes at `rate` Hz. */
uint32_t brushy_synth_length(const brushy_note_t * notes, uint32_t count, uint32_t rate);

/** Render the notes as mono samples in [-1, 1] at `rate` Hz into `out`
 *  (brushy_synth_length() samples). */
void brushy_synth_render(const brushy_note_t * notes, uint32_t count, uint32_t rate, float * out);
