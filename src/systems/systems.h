#pragma once

// ============================================================================
// Systems - ECS Logic Layer
// ============================================================================
// Systems contain the logic that operates on components.
// Components should be pure data (POD structs).
// 
// Usage:
//   1. Create a SystemManager
//   2. Add systems with addSystem<T>()
//   3. Call setup() once
//   4. Call update() and draw() each frame
//
// Example:
//   SystemManager systems;
//   systems.addSystem<Graphics2DRenderSystem>();
//   systems.addSystem<AnimationSystem>();
//   systems.setup(registry);
//   
//   // In update loop:
//   systems.update(registry, deltaTime);
//   
//   // In draw loop:
//   systems.draw(registry);

#include "base_system.h"
#include "destroy_system.h"
#include "graphics2d_system.h"
#include "mesh_render_system.h"
#include "media_render_system.h"
#include "animation_system.h"
#include "modulator_system.h"
#include "batch_renderer.h"
#include "physics_system.h"
#include "utility_render_system.h"
#include "gizmo_system.h"
#include "hardware_io_system.h"
#include "trail_system.h"
#include "effects_render_system.h"
#include "led_system.h"
#include "uv_render_system.h"
#include "layer_system.h"

// Future systems:
// #include "camera_system.h"
// #include "lighting_system.h"
