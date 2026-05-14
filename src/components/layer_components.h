#pragma once
#include "ofMain.h"

// ============================================================================
// LAYER COMPONENT
// ============================================================================
// Tags an ECS entity as belonging to a named, ordered, togglable layer.
// Systems that support layers should filter on this component and skip
// entities where !lc.visible or lc.locked.
//
// Example — rendering only visible entities on each layer in order:
//
//   auto view = registry.view<ecs::drawable, ecs::layer_component>();
//   // sort by layer index before drawing
//   view.use<ecs::layer_component>();   // or sort externally
//   for (auto [e, lc] : view.each()) {
//       if (!lc.visible) continue;
//       // ... draw e ...
//   }
// ============================================================================

namespace ecs {

struct layer_component {
    int         index   = 0;     ///< Sort / render order (lower = drawn first / bottom)
    bool        visible = true;  ///< Entities on hidden layers are skipped by layer-aware systems
    bool        locked  = false; ///< Locked layers reject edits (enforcement is per-system)
    ofColor     color;           ///< User-assigned colour for UI identification
    std::string name;            ///< Human-readable layer name
};

} // namespace ecs
