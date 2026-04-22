#include "utility_components.h"

namespace ecs {

// ============================================================================
// mask_component
// ============================================================================
// Mask operations remain in the component as they manage internal state

void mask_component::allocate(int width, int height) {
    ofFboSettings settings;
    settings.width = width;
    settings.height = height;
    settings.internalformat = GL_RGBA;
    settings.useDepth = true;
    settings.useStencil = true;
    
    maskFbo.allocate(settings);
    isMaskAllocated = true;
}

void mask_component::beginMask() {
    if (!enabled || !isMaskAllocated) return;
    
    maskFbo.begin();
    ofClear(0, 0, 0, 0);
    
    // Enable stencil buffer
    glEnable(GL_STENCIL_TEST);
    glClearStencil(0);
    glClear(GL_STENCIL_BUFFER_BIT);
    
    switch (mode) {
        case ADD:
            glStencilFunc(GL_ALWAYS, 1, 0xFF);
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
            break;
        case SUBTRACT:
            glStencilFunc(GL_ALWAYS, 0, 0xFF);
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
            break;
        case INTERSECT:
            glStencilFunc(GL_EQUAL, 1, 0xFF);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
            break;
    }
}

void mask_component::endMask() {
    if (!enabled || !isMaskAllocated) return;
    
    maskFbo.end();
    glDisable(GL_STENCIL_TEST);
}

void mask_component::applyMask() {
    if (!enabled || !isMaskAllocated) return;
    
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(invertMask ? GL_NOTEQUAL : GL_EQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
}

void mask_component::disableMask() {
    glDisable(GL_STENCIL_TEST);
}

// ============================================================================
// NOTE: Drawing and physics logic moved to Systems
// ============================================================================
// - grid_helper_component::draw() -> UtilityRenderSystem::drawGridHelper()
// - gizmo_component::draw() -> GizmoSystem::drawGizmo()
// - bounding_box_component::draw() -> UtilityRenderSystem::drawBoundingBox()
// - rigidbody_component::update() -> PhysicsSystem::updateRigidbody()

} // namespace ecs
