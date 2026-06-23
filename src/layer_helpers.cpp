#include "layer_helpers.h"
#include "entity_utils.h"
#include "components/graphics2d_components.h"

#include <algorithm>

namespace ecs {

namespace {

void ensureRelationship(entt::registry& reg, entt::entity e)
{
    if (!reg.all_of<Relationship>(e))
        reg.emplace<Relationship>(e);
}

int nextLayerIndex(entt::registry& reg)
{
    int maxIdx = -1;
    for (auto [e, lc] : reg.view<layer_component>().each()) {
        (void)e;
        maxIdx = std::max(maxIdx, lc.index);
    }
    return maxIdx + 1;
}

void collectDescendants(entt::registry& reg,
                        entt::entity e,
                        std::vector<entt::entity>& out)
{
    if (!reg.valid(e)) return;
    auto& rel = reg.get<Relationship>(e);
    entt::entity child = rel.first_child;
    while (child != entt::null) {
        collectDescendants(reg, child, out);
        out.push_back(child);
        child = reg.get<Relationship>(child).next_sibling;
    }
}

} // namespace

void linkChild(entt::registry& reg, entt::entity parent, entt::entity child)
{
    ensureRelationship(reg, child);
    auto& rel        = reg.get<Relationship>(child);
    rel.parent       = parent;
    rel.next_sibling = entt::null;
    rel.prev_sibling = entt::null;

    if (parent != entt::null && reg.valid(parent)) {
        ensureRelationship(reg, parent);
        auto& pr = reg.get<Relationship>(parent);
        pr.children_count++;
        if (pr.first_child == entt::null) {
            pr.first_child = child;
        } else {
            entt::entity last = pr.first_child;
            while (reg.get<Relationship>(last).next_sibling != entt::null)
                last = reg.get<Relationship>(last).next_sibling;
            reg.get<Relationship>(last).next_sibling = child;
            rel.prev_sibling = last;
        }
    } else {
        entt::entity lastRoot = entt::null;
        auto view = reg.view<layer_component, Relationship>();
        for (auto e : view) {
            if (e == child) continue;
            auto& er = reg.get<Relationship>(e);
            if (er.parent == entt::null && er.next_sibling == entt::null)
                lastRoot = e;
        }
        if (lastRoot != entt::null) {
            reg.get<Relationship>(lastRoot).next_sibling = child;
            rel.prev_sibling = lastRoot;
        }
    }
}

void unlinkChild(entt::registry& reg, entt::entity e)
{
    if (!reg.valid(e) || !reg.all_of<Relationship>(e)) return;
    auto& rel = reg.get<Relationship>(e);

    if (rel.prev_sibling != entt::null && reg.valid(rel.prev_sibling))
        reg.get<Relationship>(rel.prev_sibling).next_sibling = rel.next_sibling;
    if (rel.next_sibling != entt::null && reg.valid(rel.next_sibling))
        reg.get<Relationship>(rel.next_sibling).prev_sibling = rel.prev_sibling;

    if (rel.parent != entt::null && reg.valid(rel.parent)) {
        auto& pr = reg.get<Relationship>(rel.parent);
        if (pr.first_child == e) pr.first_child = rel.next_sibling;
        if (pr.children_count > 0) pr.children_count--;
    }

    rel.parent = rel.prev_sibling = rel.next_sibling = entt::null;
}

void reparent(entt::registry& reg,
              entt::entity child,
              entt::entity newParent,
              entt::entity insertBefore)
{
    if (!reg.valid(child)) return;
    if (newParent != entt::null && !reg.valid(newParent)) return;
    if (child == newParent) return;

    entt::entity check = newParent;
    while (check != entt::null && reg.valid(check)) {
        if (check == child) return;
        check = reg.get<Relationship>(check).parent;
    }

    unlinkChild(reg, child);

    if (insertBefore != entt::null && reg.valid(insertBefore)) {
        auto& ibRel    = reg.get<Relationship>(insertBefore);
        auto& childRel = reg.get<Relationship>(child);

        childRel.parent       = ibRel.parent;
        childRel.prev_sibling = ibRel.prev_sibling;
        childRel.next_sibling = insertBefore;

        if (ibRel.prev_sibling != entt::null)
            reg.get<Relationship>(ibRel.prev_sibling).next_sibling = child;
        if (childRel.parent != entt::null) {
            auto& pr = reg.get<Relationship>(childRel.parent);
            if (pr.first_child == insertBefore) pr.first_child = child;
            pr.children_count++;
        }
        ibRel.prev_sibling = child;
    } else {
        linkChild(reg, newParent, child);
    }
}

void destroySubtree(entt::registry& reg, entt::entity root)
{
    if (!reg.valid(root)) return;
    std::vector<entt::entity> descendants;
    collectDescendants(reg, root, descendants);
    unlinkChild(reg, root);
    for (auto it = descendants.rbegin(); it != descendants.rend(); ++it) {
        if (reg.valid(*it)) reg.destroy(*it);
    }
    if (reg.valid(root)) reg.destroy(root);
}

entt::entity findFirstLayerRoot(entt::registry& reg)
{
    return findFirstLayerRoot(const_cast<const entt::registry&>(reg));
}

entt::entity findFirstLayerRoot(const entt::registry& reg)
{
    auto view = reg.view<const layer_component, const Relationship>();
    for (auto e : view) {
        const auto& rel = reg.get<const Relationship>(e);
        if (rel.parent == entt::null && rel.prev_sibling == entt::null)
            return e;
    }
    return entt::null;
}

bool layerEntityVisible(entt::registry& reg, entt::entity layerEntity)
{
    if (!reg.valid(layerEntity) || !reg.all_of<layer_component>(layerEntity))
        return false;
    return reg.get<layer_component>(layerEntity).visible;
}

bool entityEffectivelyVisible(entt::registry& reg, entt::entity e)
{
    if (!reg.valid(e)) return false;

  entt::entity cur = e;
    while (cur != entt::null && reg.valid(cur)) {
        if (reg.all_of<layer_component>(cur)) {
            if (!reg.get<layer_component>(cur).visible) return false;
        }
        if (!reg.all_of<Relationship>(cur)) break;
        cur = reg.get<Relationship>(cur).parent;
    }
    return true;
}

std::string entityDisplayName(entt::registry& reg, entt::entity e)
{
    if (!reg.valid(e)) return "Entity";
    if (reg.all_of<layer_component>(e))
        return reg.get<layer_component>(e).name;
    if (reg.all_of<tag_component>(e))
        return reg.get<tag_component>(e).tag;
    if (reg.all_of<node_component>(e))
        return reg.get<node_component>(e).getName();
    if (reg.all_of<path_component>(e)) return "Path";
    if (reg.all_of<image_component>(e)) return "Image";
    return "Entity " + getStringID(e);
}

bool Layer::valid() const
{
    return reg_ && reg_->valid(entity_) && reg_->all_of<layer_component>(entity_);
}

layer_component& Layer::component()
{
    return reg_->get<layer_component>(entity_);
}

const layer_component& Layer::component() const
{
    return reg_->get<layer_component>(entity_);
}

std::string Layer::name() const
{
    return valid() ? component().name : std::string();
}

bool Layer::visible() const
{
    return valid() && component().visible;
}

bool Layer::locked() const
{
    return valid() && component().locked;
}

ofColor Layer::color() const
{
    return valid() ? component().color : ofColor();
}

Layer createLayer(entt::registry& reg,
                  const std::string& name,
                  entt::entity parentLayer)
{
    auto e = reg.create();

    layer_component lc;
    lc.index   = nextLayerIndex(reg);
    lc.name    = name.empty()
        ? ("Layer " + std::to_string(lc.index + 1))
        : name;
    lc.color   = ofColor(120, 140, 180);
    lc.visible = true;
    lc.locked  = false;

    reg.emplace<layer_component>(e, lc);
    ensureRelationship(reg, e);
    ensureSelectable(reg, e);

    if (parentLayer != entt::null && reg.valid(parentLayer))
        linkChild(reg, parentLayer, e);
    else
        linkChild(reg, entt::null, e);

    return Layer(reg, e);
}

entt::entity addToLayer(entt::registry& reg, Layer layer, entt::entity child)
{
    if (!layer.valid() || !reg.valid(child)) return entt::null;
    unlinkChild(reg, child);
    linkChild(reg, layer.entity(), child);
    ensureRelationship(reg, child);
    ensureSelectable(reg, child);
    return child;
}

} // namespace ecs
