#pragma once

// Optional systems (everything under src/systems/ except core + TransformSystem).
// Core (via ofxEnTTKit.h): base_system, destroy_system, layer_system, TransformSystem.
//
// Convenience umbrella — include only the headers you need, or this file for the full set.

#include "systems/IShapeRenderer.h"
#include "systems/graphics2d_system.h"
#include "systems/mesh_render_system.h"
#include "systems/batch_renderer.h"
#include "systems/uv_render_system.h"
#include "systems/effects_render_system.h"
#include "systems/media_render_system.h"
#include "systems/generator_render_system.h"
#include "systems/animation_system.h"
#include "systems/modulator_system.h"
#include "systems/physics_system.h"
#include "systems/utility_render_system.h"
#include "systems/gizmo_system.h"
#include "systems/trail_system.h"
#include "systems/hardware_io_system.h"
#include "systems/input_system.h"
#include "systems/led_system.h"
