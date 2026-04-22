#pragma once
#include "ofMain.h"
#include <entt.hpp>

// Forward declare

// ============================================================================
// UTILITY COMPONENTS (Helpers & Tools)
// ============================================================================

namespace ecs {

// ============================================================================
// Grid Helper Component
// ============================================================================

struct grid_helper_component {
    float size;
    int divisions;
    
    ofColor centerLineColor;
    ofColor gridColor;
    
    bool showXY;
    bool showXZ;
    bool showYZ;
    
    bool showAxes;
    float axisLength;
    
    grid_helper_component()
        : size(1000.0f)
        , divisions(10)
        , centerLineColor(100, 100, 100, 255)
        , gridColor(50, 50, 50, 255)
        , showXY(false)
        , showXZ(true)
        , showYZ(false)
        , showAxes(true)
        , axisLength(100.0f)
    {}
    
    // Drawing is handled by UtilityRenderSystem
};

// ============================================================================
// Gizmo Component
// ============================================================================

struct gizmo_component {
    enum GizmoMode {
        TRANSLATE,
        ROTATE,
        SCALE
    };
    
    GizmoMode mode;
    bool enabled;
    bool localSpace;  // vs world space
    
    // Visual
    float size;
    ofColor xColor;
    ofColor yColor;
    ofColor zColor;
    ofColor selectedColor;
    
    // Interaction
    enum Axis { NONE, X, Y, Z, XY, YZ, XZ, XYZ };
    Axis selectedAxis;
    bool dragging;
    
    gizmo_component()
        : mode(TRANSLATE)
        , enabled(true)
        , localSpace(false)
        , size(50.0f)
        , xColor(255, 0, 0, 200)
        , yColor(0, 255, 0, 200)
        , zColor(0, 0, 255, 200)
        , selectedColor(255, 255, 0, 255)
        , selectedAxis(NONE)
        , dragging(false)
    {}
    
    // Drawing is handled by GizmoSystem
};

// ============================================================================
// Bounding Box Component
// ============================================================================

struct bounding_box_component {
    glm::vec3 min;
    glm::vec3 max;
    
    bool autoUpdate;
    bool visible;
    ofColor color;
    
    bounding_box_component()
        : min(0, 0, 0)
        , max(0, 0, 0)
        , autoUpdate(false)
        , visible(true)
        , color(255, 255, 0, 128)
    {}
    
    bounding_box_component(const glm::vec3& minCorner, const glm::vec3& maxCorner)
        : min(minCorner)
        , max(maxCorner)
        , autoUpdate(false)
        , visible(true)
        , color(255, 255, 0, 128)
    {}
    
    glm::vec3 getCenter() const {
        return (min + max) * 0.5f;
    }
    
    glm::vec3 getSize() const {
        return max - min;
    }
    
    float getWidth() const { return max.x - min.x; }
    float getHeight() const { return max.y - min.y; }
    float getDepth() const { return max.z - min.z; }
    
    bool contains(const glm::vec3& point) const {
        return point.x >= min.x && point.x <= max.x &&
               point.y >= min.y && point.y <= max.y &&
               point.z >= min.z && point.z <= max.z;
    }
    
    bool intersects(const bounding_box_component& other) const {
        return !(max.x < other.min.x || min.x > other.max.x ||
                 max.y < other.min.y || min.y > other.max.y ||
                 max.z < other.min.z || min.z > other.max.z);
    }
    
    void expand(const glm::vec3& point) {
        min = glm::min(min, point);
        max = glm::max(max, point);
    }
    
    void expand(const bounding_box_component& other) {
        min = glm::min(min, other.min);
        max = glm::max(max, other.max);
    }
    
    // Drawing is handled by UtilityRenderSystem
};

// ============================================================================
// Mask Component
// ============================================================================

struct mask_component {
    enum MaskMode {
        ADD,
        SUBTRACT,
        INTERSECT
    };
    
    MaskMode mode;
    bool enabled;
    bool invertMask;
    
    ofFbo maskFbo;
    bool isMaskAllocated;
    
    mask_component()
        : mode(ADD)
        , enabled(true)
        , invertMask(false)
        , isMaskAllocated(false)
    {}
    
    void allocate(int width, int height);
    void beginMask();
    void endMask();
    void applyMask();
    void disableMask();
};

// ============================================================================
// Rigidbody Component (Basic Physics)
// ============================================================================

struct rigidbody_component {
    glm::vec3 velocity;
    glm::vec3 acceleration;
    glm::vec3 angularVelocity;
    glm::vec3 angularAcceleration;

    float mass;
    float drag;
    float angularDrag;
    float restitution;  // Bounciness (0 = no bounce, 1 = perfect bounce)
    float friction;     // Contact friction (used by Bullet; simple PhysicsSystem ignores)

    bool useGravity;
    bool isKinematic;  // If true, not affected by physics
    bool useBullet;   // If true, ofxBullet drives this body (collision shape from mesh_component when present)
    bool freezePositionX;
    bool freezePositionY;
    bool freezePositionZ;
    bool freezeRotationX;
    bool freezeRotationY;
    bool freezeRotationZ;

    rigidbody_component()
        : velocity(0, 0, 0)
        , acceleration(0, 0, 0)
        , angularVelocity(0, 0, 0)
        , angularAcceleration(0, 0, 0)
        , mass(1.0f)
        , drag(0.01f)
        , angularDrag(0.05f)
        , restitution(0.5f)
        , friction(0.5f)
        , useGravity(true)
        , isKinematic(false)
        , useBullet(false)
        , freezePositionX(false)
        , freezePositionY(false)
        , freezePositionZ(false)
        , freezeRotationX(false)
        , freezeRotationY(false)
        , freezeRotationZ(false)
    {}

    // Physics: PhysicsSystem (simple) or ofxBullet (when useBullet and app has Bullet world).
    // Collision shape when using Bullet: inferred from mesh_component (primitiveType + width/height/depth/radius) when present.
    // Use PhysicsSystem::applyForce/applyImpulse/applyTorque for simple backend.
};


} // namespace ecs
