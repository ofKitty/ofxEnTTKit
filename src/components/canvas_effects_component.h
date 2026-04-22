#pragma once

#include "ofMain.h"
#include <entt.hpp>
#include <vector>
#include "filter_components.h"

namespace ecs {

// ============================================================================
// Canvas Effects Component
// ============================================================================
// Attaches to an entity with fbo_component to apply post-processing effects.
// Filters are applied in order from the effect stack.
//
// Usage:
// 1. Add this component to an entity that has fbo_component
// 2. Add filter entities (tint_filter_component, blur_filter_component, etc.)
// 3. Reference those entities in the effectStack
// 4. Call applyEffects() after drawing canvas contents
// ============================================================================

struct canvas_effects_component {
    // List of entities that have filter components
    // Filters are applied in order (first to last)
    std::vector<entt::entity> effectStack;
    
    // If true, filters are applied automatically during canvas render
    bool autoApply = true;
    
    // If true, a ping-pong buffer is used for multi-pass effects
    bool usePingPong = true;
    
    // Temporary FBO for ping-pong rendering
    ofFbo pingPongFbo;
    bool pingPongAllocated = false;
    
    // Add a filter entity to the stack
    void addEffect(entt::entity filterEntity);
    
    // Remove a filter entity from the stack
    void removeEffect(entt::entity filterEntity);
    
    // Move a filter up in the stack (earlier in processing)
    void moveUp(int index);
    
    // Move a filter down in the stack (later in processing)
    void moveDown(int index);
    
    // Clear all effects
    void clearEffects() { effectStack.clear(); }
    
    // Get effect count
    int getEffectCount() const { return (int)effectStack.size(); }
    
    // Ensure ping-pong FBO matches the main FBO size
    void ensurePingPong(int width, int height, int internalFormat = GL_RGBA);
    
    // Apply all enabled filters in the stack to the given FBO
    // Returns the FBO that contains the final result (either source or pingPong)
    ofFbo& applyEffects(entt::registry& registry, ofFbo& sourceFbo);
};

// Serialization helpers (declaration only, implemented in .cpp)
ofJson serializeCanvasEffects(const canvas_effects_component& c);
void deserializeCanvasEffects(canvas_effects_component& c, const ofJson& j);

} // namespace ecs
