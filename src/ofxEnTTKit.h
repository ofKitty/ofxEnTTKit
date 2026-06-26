#pragma once

// ============================================================================
// ofxEnTTKit — core ECS (EnTT + basic components & systems)
// ============================================================================
// Depends on: ofxEnTT
//
// Full kit (all optional components + systems):
//   #include "ofxEnTTKit_all.h"
// Or pick layers / individual headers under src/components/ and src/systems/.
//
// Namespace: ecs::
// ============================================================================

#include <entt.hpp>

#include "entity_utils.h"

#include "components/hierarchy_components.h"
#include "components/base_components.h"
#include "components/layer_components.h"
#include "components/state_components.h"
#include "layer_helpers.h"

#include "ofxNode.h"
#include "entity_handle.h"
#include "painter.h"

#include "systems/systems_core.h"
#include "systems/TransformSystem.h"
