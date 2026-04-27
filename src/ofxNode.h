#pragma once

#include <entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <functional>
#include <string>

// Forward declarations
namespace ecs {
struct LocalTransform;
struct GlobalTransform;
struct Relationship;
} // namespace ecs

// ============================================================================
// ofxNode — ECS-native scene node handle
// ============================================================================
// Wraps an entt::entity that owns ecs::Relationship, ecs::LocalTransform,
// and ecs::GlobalTransform components.
//
// ofxNode is a lightweight handle (entity id + registry pointer) — it is
// copyable and movable. The actual data lives in the ECS components.
//
// Usage:
//   ofxNode cam(registry, "Camera");
//   cam.setPosition({0, 100, 300});
//   cam.setParent(sceneRoot);
//
// TransformSystem must be called once per frame to recompute GlobalTransform
// for all nodes before reading getGlobalTransform().
// ============================================================================

class ofxNode {
public:
    /// Creates a new entity and emplaces Relationship + LocalTransform + GlobalTransform.
    explicit ofxNode(entt::registry& reg, const std::string& name = "Node");

    /// Wrap an existing entity (entity must already have the required components).
    static ofxNode fromEntity(entt::registry& reg, entt::entity e);

    ofxNode(const ofxNode&)            = default;
    ofxNode& operator=(const ofxNode&) = default;
    ofxNode(ofxNode&&)                 = default;
    ofxNode& operator=(ofxNode&&)      = default;

    // -------------------------------------------------------------------------
    // Transform (local space)
    // -------------------------------------------------------------------------

    void      setPosition(const glm::vec3& pos);
    glm::vec3 getPosition() const;

    void      setOrientation(const glm::quat& q);
    glm::quat getOrientation() const;

    void      setScale(const glm::vec3& s);
    glm::vec3 getScale() const;

    /// World-space transform matrix — valid after TransformSystem::update() runs.
    glm::mat4 getGlobalTransform() const;

    // -------------------------------------------------------------------------
    // Hierarchy
    // -------------------------------------------------------------------------

    /// Attach this node as the last child of parent.
    void setParent(ofxNode& parent);

    /// Detach this node from its current parent (becomes a root node).
    void clearParent();

    /// Returns a handle to the parent, or an invalid ofxNode if root.
    ofxNode  getParent() const;
    bool     hasParent() const;

    /// Iterate direct children (non-recursive).
    void forEachChild(const std::function<void(ofxNode)>& fn) const;

    /// Depth-first pre-order traversal of this node and all descendants.
    void forEachInBranch(const std::function<void(ofxNode)>& fn) const;

    std::size_t childrenCount() const;

    // -------------------------------------------------------------------------
    // Metadata
    // -------------------------------------------------------------------------

    const std::string& getName() const { return m_name; }
    void               setName(const std::string& n)  { m_name = n; }

    // -------------------------------------------------------------------------
    // Escape hatch
    // -------------------------------------------------------------------------

    entt::entity    entity()   const { return m_entity; }
    entt::registry* registry() const { return m_reg; }
    bool            valid()    const { return m_reg && m_reg->valid(m_entity); }

    bool operator==(const ofxNode& o) const { return m_entity == o.m_entity; }
    bool operator!=(const ofxNode& o) const { return m_entity != o.m_entity; }

private:
    ofxNode() = default;

    entt::entity    m_entity {entt::null};
    entt::registry* m_reg    {nullptr};
    std::string     m_name;
};
