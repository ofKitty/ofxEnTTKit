#pragma once

// Optional components (everything under src/components/ except the core set in ofxEnTTKit.h).
// Core (always via ofxEnTTKit.h): hierarchy, base, layer, state.
//
// Convenience umbrella — include only the headers you need, or this file for the full set.

#include "components/graphics2d_components.h"
#include "components/shape2d_component.h"
#include "components/rendering_components.h"
#include "components/animation_components.h"
#include "components/modulator_component.h"
#include "components/easing.h"
#include "components/eased_pulse_component.h"
#include "components/music_components.h"
#include "components/filter_components.h"
#include "components/draw_filter_components.h"
#include "components/canvas_effects_component.h"
#include "components/generator_components.h"
#include "components/paint_components.h"
#include "components/input_components.h"
#include "components/hardware_components.h"
#include "components/sensor_components.h"
#include "components/audio_source_component.h"
#include "components/midi_source_component.h"
#include "components/led_components.h"
#include "components/utility_components.h"
#include "components/color_band_component.h"
#include "components/trace_components.h"
#include "components/webcam_component.h"
