#include "ofxNode.h"
#include "components/hierarchy_components.h"

// ============================================================================
// ofxNode implementation
// ============================================================================

ofxNode::ofxNode(entt::registry& reg, const std::string& name)
    : m_reg(&reg)
    , m_name(name)
{
    m_entity = reg.create();
    reg.emplace<ecs::Relationship>(m_entity);
    reg.emplace<ecs::LocalTransform>(m_entity);
    reg.emplace<ecs::GlobalTransform>(m_entity);
}

ofxNode ofxNode::fromEntity(entt::registry& reg, entt::entity e)
{
    ofxNode n;
    n.m_entity = e;
    n.m_reg    = &reg;
    return n;
}

// -------------------------------------------------------------------------
// Transform
// -------------------------------------------------------------------------

void ofxNode::setPosition(const glm::vec3& pos)
{
    m_reg->get<ecs::LocalTransform>(m_entity).position = pos;
}

glm::vec3 ofxNode::getPosition() const
{
    return m_reg->get<ecs::LocalTransform>(m_entity).position;
}

void ofxNode::setOrientation(const glm::quat& q)
{
    m_reg->get<ecs::LocalTransform>(m_entity).orientation = q;
}

glm::quat ofxNode::getOrientation() const
{
    return m_reg->get<ecs::LocalTransform>(m_entity).orientation;
}

void ofxNode::setScale(const glm::vec3& s)
{
    m_reg->get<ecs::LocalTransform>(m_entity).scale = s;
}

glm::vec3 ofxNode::getScale() const
{
    return m_reg->get<ecs::LocalTransform>(m_entity).scale;
}

glm::mat4 ofxNode::getGlobalTransform() const
{
    return m_reg->get<ecs::GlobalTransform>(m_entity).matrix;
}

// -------------------------------------------------------------------------
// Hierarchy helpers
// -------------------------------------------------------------------------

void ofxNode::setParent(ofxNode& parent)
{
    if (!valid() || !parent.valid()) return;
    if (m_entity == parent.m_entity) return;

    // Detach from old parent first
    clearParent();

    auto& childRel  = m_reg->get<ecs::Relationship>(m_entity);
    auto& parentRel = m_reg->get<ecs::Relationship>(parent.m_entity);

    childRel.parent = parent.m_entity;

    // Append to the end of the parent's child list
    if (parentRel.first_child == entt::null) {
        parentRel.first_child = m_entity;
        childRel.prev_sibling = entt::null;
        childRel.next_sibling = entt::null;
    } else {
        // Walk to last sibling
        entt::entity last = parentRel.first_child;
        while (m_reg->get<ecs::Relationship>(last).next_sibling != entt::null)
            last = m_reg->get<ecs::Relationship>(last).next_sibling;

        m_reg->get<ecs::Relationship>(last).next_sibling = m_entity;
        childRel.prev_sibling = last;
        childRel.next_sibling = entt::null;
    }

    parentRel.children_count++;
}

void ofxNode::clearParent()
{
    if (!valid()) return;
    auto& rel = m_reg->get<ecs::Relationship>(m_entity);
    if (rel.parent == entt::null) return;

    auto& parentRel = m_reg->get<ecs::Relationship>(rel.parent);

    // Stitch siblings together
    if (rel.prev_sibling != entt::null)
        m_reg->get<ecs::Relationship>(rel.prev_sibling).next_sibling = rel.next_sibling;
    else
        parentRel.first_child = rel.next_sibling; // we were the first child

    if (rel.next_sibling != entt::null)
        m_reg->get<ecs::Relationship>(rel.next_sibling).prev_sibling = rel.prev_sibling;

    if (parentRel.children_count > 0)
        parentRel.children_count--;

    rel.parent       = entt::null;
    rel.prev_sibling = entt::null;
    rel.next_sibling = entt::null;
}

ofxNode ofxNode::getParent() const
{
    auto& rel = m_reg->get<ecs::Relationship>(m_entity);
    return fromEntity(*m_reg, rel.parent);
}

bool ofxNode::hasParent() const
{
    return m_reg->get<ecs::Relationship>(m_entity).parent != entt::null;
}

void ofxNode::forEachChild(const std::function<void(ofxNode)>& fn) const
{
    entt::entity cur = m_reg->get<ecs::Relationship>(m_entity).first_child;
    while (cur != entt::null) {
        fn(fromEntity(*m_reg, cur));
        cur = m_reg->get<ecs::Relationship>(cur).next_sibling;
    }
}

void ofxNode::forEachInBranch(const std::function<void(ofxNode)>& fn) const
{
    fn(fromEntity(*m_reg, m_entity));
    forEachChild([&fn](ofxNode child) {
        child.forEachInBranch(fn);
    });
}

std::size_t ofxNode::childrenCount() const
{
    return m_reg->get<ecs::Relationship>(m_entity).children_count;
}
