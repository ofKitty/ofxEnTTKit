#pragma once

// ============================================================================
// ofxEnTTKit - ECS components & systems for openFrameworks (built on EnTT)
// ============================================================================
// Depends on: ofxEnTT (for the EnTT library itself)
// EnTT library: https://github.com/skypjack/entt
//
// This addon provides, on top of EnTT:
// - Core ECS components for common OF types (mesh, camera, image, etc.)
// - Rendering components (materials, lights, shaders)
// - 2D graphics components (paths, polylines, shapes)
// - Animation components (tween, particles)
// - Hardware I/O components (serial, OSC)
// - LED components (fixtures, mapping)
// - Utility components (grids, gizmos)
// - Systems that operate on these components
//
// Namespaces:
// - ecs::  Contains all components and systems
//
// Components use _component suffix (e.g., ecs::camera_component)
// Systems use descriptive names (e.g., ecs::Graphics2DRender)
//
// Inspector / ImGui property drawers live in ofxEnTTInspector (type-driven).
//
// Shipped-kit “Add Component” menu rows (labels + add/remove hooks) are declared
// in component_editor_registration.h — pulled in below so one #include gets
// both ECS definitions and ecs::registerKitComponentMenu(...) for shells.
// ============================================================================

// EnTT core
#include <entt.hpp>

// Entity utilities
#include "entity_utils.h"

// Components
#include "components/hierarchy_components.h"
#include "components/base_components.h"
#include "components/rendering_components.h"
#include "components/graphics2d_components.h"
#include "components/shape2d_component.h"
#include "components/animation_components.h"
#include "components/hardware_components.h"
#include "components/led_components.h"
#include "components/utility_components.h"
#include "components/modulator_component.h"
#include "components/swatch_components.h"
#include "components/easing.h"
#include "components/filter_components.h"
#include "components/generator_components.h"
#include "components/draw_filter_components.h"
#include "components/eased_pulse_component.h"
#include "components/state_components.h"
#include "components/audio_source_component.h"
#include "components/midi_source_component.h"
#include "components/canvas_effects_component.h"

// Default picker rows for shipped ecs::* types (ofxKit Properties, …)
#include "component_editor_registration.h"

// ECS-native node handle
#include "ofxNode.h"

// Systems
#include "systems/systems.h"
#include "systems/TransformSystem.h"
