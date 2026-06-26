#pragma once

// ============================================================================
// ecs::Painter — authoring sugar for 2D shapes AND 3D meshes
// ============================================================================
// One stateful builder that emits persistent ECS entities into a single
// registry, with the boilerplate handled for you: every primitive gets a
// render_component + selectable_component (and, in 2D, is parented to a layer),
// so the result is fully selectable / editable in the ofxKit editor.
//
// There is no separate 2D vs 3D engine — "2D" is just the ortho view of the
// same registry. The Painter holds ONE shared style + transform state, so you
// can draw a rect and a sphere from the same object without juggling two
// states. Style maps across dimensions:
//
//      fill   -> 2D fillColor/filled      3D mesh.color + drawFaces
//      stroke -> 2D strokeColor/stroked   3D drawWireframe
//
// Two front-ends, one engine:
//
//   Way A — chaining Painter (state lives on the object):
//       ecs::Painter paint(reg);
//       paint.fill(ofColor(80,160,240)).box("Box", {-200,0,0}, 100);
//       auto rect = paint.rect(80, 80, 200, 200);   // returns ecs::Entity
//
//   Way B — free "immediate" functions:
//       layer.begin();  // or ecs::beginPainter(reg)
//         ecs::fill(ofColor(80,160,240));
//         ecs::drawRect(80, 80, 200, 200);
//       layer.end();
//
// Two stacks, push()/pop() save STYLE (cf. ofPushStyle), while
// pushMatrix()/popMatrix() save the TRANSFORM (cf. ofPushMatrix). The current
// matrix places primitives in both 2D (LocalTransform) and 3D (node_component).
//
// Pure data — no ImGui, no rendering here. 2D shapes draw via ecs::Graphics2DRender
// (e.g. ofkitty::drawLayerContents); meshes via ecs::drawMeshes.
// ============================================================================

#include "ofMain.h"
#include "entity_handle.h"   // ecs::Entity, ecs::spawn, eMeshPrimitiveType, node/mesh comps
#include "layer_helpers.h"   // ecs::Layer, createLayer, addToLayer

#include <glm/gtc/matrix_transform.hpp>   // glm::translate/rotate/scale used inline below
#include <vector>

namespace ecs {

// ----------------------------------------------------------------------------
// ShapeStyle — the "preset" applied to each primitive a Painter creates.
// ----------------------------------------------------------------------------
struct ShapeStyle {
    ofColor fill        { 255, 255, 255, 255 };
    bool    filled      { true };
    ofColor stroke      { 0, 0, 0, 255 };
    float   strokeWidth { 1.0f };
    bool    stroked     { false };
    bool    selectable  { true };   // add selectable_component
    bool    visible     { true };   // render_component.visible
};

// ----------------------------------------------------------------------------
// Painter — chaining factory for 2D shapes + 3D meshes (Way A, the engine).
// ----------------------------------------------------------------------------
class Painter {
public:
    explicit Painter(entt::registry& reg);       // 3D / layer-less
    explicit Painter(const Layer& layer);         // 2D, parents shapes to the layer
    Painter(entt::registry& reg, entt::entity layer);

    // -- style (chaining) ----------------------------------------------------
    Painter& fill(const ofColor& c)                  { style_.fill = c; style_.filled = true; return *this; }
    Painter& noFill()                                { style_.filled = false; return *this; }
    Painter& stroke(const ofColor& c, float w = 1.f) { style_.stroke = c; style_.strokeWidth = w; style_.stroked = true; return *this; }
    Painter& noStroke()                              { style_.stroked = false; return *this; }
    Painter& style(const ShapeStyle& s)              { style_ = s; return *this; }
    Painter& push()                                  { stack_.push_back(style_); return *this; }
    Painter& pop()                                   { if (!stack_.empty()) { style_ = stack_.back(); stack_.pop_back(); } return *this; }

    const ShapeStyle& currentStyle() const           { return style_; }

    // -- transform stack (chaining; cf. ofPushMatrix/ofTranslate) ------------
    Painter& pushMatrix()                            { matrixStack_.push_back(matrix_); return *this; }
    Painter& popMatrix()                             { if (!matrixStack_.empty()) { matrix_ = matrixStack_.back(); matrixStack_.pop_back(); } return *this; }
    Painter& resetMatrix()                           { matrix_ = glm::mat4(1.f); return *this; }
    Painter& translate(const glm::vec3& t)           { matrix_ = glm::translate(matrix_, t); return *this; }
    Painter& translate(float x, float y, float z = 0.f) { return translate(glm::vec3(x, y, z)); }
    Painter& rotateDeg(float deg, const glm::vec3& axis) { matrix_ = glm::rotate(matrix_, glm::radians(deg), axis); return *this; }
    Painter& scale(const glm::vec3& s)               { matrix_ = glm::scale(matrix_, s); return *this; }
    Painter& scale(float s)                          { return scale(glm::vec3(s, s, s)); }

    const glm::mat4& currentMatrix() const           { return matrix_; }

    // -- 2D shapes (auto render + selectable, parented to the layer) ---------
    Entity rect(float x, float y, float w, float h);
    Entity circle(float x, float y, float r);
    Entity ellipse(float x, float y, float w, float h);
    Entity line(glm::vec2 a, glm::vec2 b);
    Entity triangle(glm::vec2 a, glm::vec2 b, glm::vec2 c);
    Entity path(const ofPath& p);

    // -- 3D meshes (auto node + render + selectable; placed by current matrix)
    // pos defaults to the origin, so box("Box") lands at the current transform.
    Entity mesh(eMeshPrimitiveType type, const std::string& name, glm::vec3 pos = {});
    Entity box(const std::string& name, glm::vec3 pos = {}, float size = 100.f);
    Entity sphere(const std::string& name, glm::vec3 pos = {}, float radius = 55.f);
    Entity cone(const std::string& name, glm::vec3 pos = {}, float radius = 45.f, float height = 130.f);
    Entity cylinder(const std::string& name, glm::vec3 pos = {}, float radius = 40.f, float height = 120.f);
    Entity icosphere(const std::string& name, glm::vec3 pos = {}, float radius = 60.f);
    Entity plane(const std::string& name, glm::vec3 pos = {}, float w = 200.f, float h = 200.f);

    // -- one-shot keyword-arg overrides (style applied for this call only) ---
    Entity rect(float x, float y, float w, float h, const ShapeStyle& s)     { return scoped(s, [&]{ return rect(x, y, w, h); }); }
    Entity circle(float x, float y, float r, const ShapeStyle& s)            { return scoped(s, [&]{ return circle(x, y, r); }); }
    Entity ellipse(float x, float y, float w, float h, const ShapeStyle& s)  { return scoped(s, [&]{ return ellipse(x, y, w, h); }); }
    Entity line(glm::vec2 a, glm::vec2 b, const ShapeStyle& s)                { return scoped(s, [&]{ return line(a, b); }); }
    Entity triangle(glm::vec2 a, glm::vec2 b, glm::vec2 c, const ShapeStyle& s){ return scoped(s, [&]{ return triangle(a, b, c); }); }
    Entity path(const ofPath& p, const ShapeStyle& s)                        { return scoped(s, [&]{ return path(p); }); }

private:
    entt::registry*         reg_   { nullptr };
    entt::entity            layer_ { entt::null };
    ShapeStyle              style_ {};
    std::vector<ShapeStyle> stack_;
    glm::mat4               matrix_ { 1.f };
    std::vector<glm::mat4>  matrixStack_;

    /// Apply the current fill/stroke style to any 2D component exposing the
    /// shared fillColor / strokeColor / strokeWidth / filled / stroked fields.
    template <class C>
    void applyStyle(C& c) const {
        c.filled      = style_.filled;
        c.fillColor   = style_.fill;
        c.stroked     = style_.stroked;
        c.strokeColor = style_.stroke;
        c.strokeWidth = style_.strokeWidth;
    }

    /// Add render + selectable; parent to the layer when one is set (2D).
    Entity finishCommon(Entity e);
    /// finishCommon + attach a 2D LocalTransform when the matrix is non-identity.
    Entity finish2D(Entity e);
    /// Place the entity's node by (matrix * translate(pos)), then finishCommon.
    Entity finishMesh(Entity e, glm::vec3 pos);

    template <class Fn>
    Entity scoped(const ShapeStyle& s, Fn&& fn) {
        push(); style(s);
        Entity e = fn();
        pop();
        return e;
    }
};

// ----------------------------------------------------------------------------
// Way B — free "immediate" API over a current Painter.
// Layer::begin() (2D) or beginPainter(reg) (any) sets the current painter.
// ----------------------------------------------------------------------------
void     beginPainter(const Layer& layer);
void     beginPainter(entt::registry& reg);
void     endPainter();
bool     hasCurrentPainter();
Painter& currentPainter();          // valid only between begin()/end()

void fill(const ofColor& c);
void noFill();
void stroke(const ofColor& c, float w = 1.f);
void noStroke();

void pushMatrix();
void popMatrix();
void resetMatrix();
void translate(const glm::vec3& t);
void rotateDeg(float deg, const glm::vec3& axis);
void scale(const glm::vec3& s);
void scale(float s);

// 2D
Entity drawRect(float x, float y, float w, float h);
Entity drawCircle(float x, float y, float r);
Entity drawEllipse(float x, float y, float w, float h);
Entity drawLine(glm::vec2 a, glm::vec2 b);
Entity drawTriangle(glm::vec2 a, glm::vec2 b, glm::vec2 c);
Entity drawPath(const ofPath& p);

// 3D
Entity drawBox(const std::string& name, glm::vec3 pos = {}, float size = 100.f);
Entity drawSphere(const std::string& name, glm::vec3 pos = {}, float radius = 55.f);
Entity drawCone(const std::string& name, glm::vec3 pos = {}, float radius = 45.f, float height = 130.f);
Entity drawCylinder(const std::string& name, glm::vec3 pos = {}, float radius = 40.f, float height = 120.f);
Entity drawIcosphere(const std::string& name, glm::vec3 pos = {}, float radius = 60.f);
Entity drawPlane(const std::string& name, glm::vec3 pos = {}, float w = 200.f, float h = 200.f);

} // namespace ecs
