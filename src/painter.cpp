#include "painter.h"

#include "components/graphics2d_components.h"   // rectangle/circle/ellipse/line/triangle/path
#include "components/base_components.h"          // render/selectable, node/mesh, eMeshPrimitiveType
#include "components/hierarchy_components.h"      // LocalTransform

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <memory>

namespace ecs {

namespace {

/// Decompose an affine (translate/rotate/scale, no shear) matrix into TRS.
/// Done by hand to avoid GLM's quaternion-conjugate ambiguity in decompose().
void decomposeTRS(const glm::mat4& m, glm::vec3& t, glm::quat& r, glm::vec3& s) {
    t = glm::vec3(m[3]);
    const glm::vec3 c0(m[0]), c1(m[1]), c2(m[2]);
    s = { glm::length(c0), glm::length(c1), glm::length(c2) };
    const glm::mat3 rot(
        s.x > 0.f ? c0 / s.x : glm::vec3(1, 0, 0),
        s.y > 0.f ? c1 / s.y : glm::vec3(0, 1, 0),
        s.z > 0.f ? c2 / s.z : glm::vec3(0, 0, 1));
    r = glm::quat_cast(rot);
}

void applyMeshStyleTo(mesh_component& m, const ShapeStyle& style) {
    m.drawFaces     = style.filled;
    m.drawWireframe = style.stroked;
    if (style.filled)       m.color = style.fill;
    else if (style.stroked) m.color = style.stroke;
}

} // namespace

// ----------------------------------------------------------------------------
// Painter — construction
// ----------------------------------------------------------------------------

Painter::Painter(entt::registry& reg)
    : reg_(&reg), layer_(entt::null) {}

Painter::Painter(const Layer& layer)
    : reg_(&layer.registry()), layer_(layer.entity()) {
    // Sensible default: shapes inherit the layer's colour unless overridden, so
    // a bare drawRect(...) is visible and tinted like the rest of the layer.
    style_.fill = layer.color();
}

Painter::Painter(entt::registry& reg, entt::entity layer)
    : reg_(&reg), layer_(layer) {}

// ----------------------------------------------------------------------------
// Painter — finish helpers
// ----------------------------------------------------------------------------

Entity Painter::finishCommon(Entity e) {
    e.add<render_component>(0, false, style_.visible);
    if (style_.selectable) e.add<selectable_component>();
    if (layer_ != entt::null) addToLayer(*reg_, Layer(*reg_, layer_), e);
    return e;
}

Entity Painter::finish2D(Entity e) {
    // Only attach a transform when the cursor has actually moved, so untouched
    // 2D sketches keep their component-local x/y coordinates verbatim.
    if (matrix_ != glm::mat4(1.f)) {
        LocalTransform lt;
        decomposeTRS(matrix_, lt.position, lt.orientation, lt.scale);
        e.add<LocalTransform>(lt);
    }
    return finishCommon(e);
}

// ----------------------------------------------------------------------------
// Painter — 2D shapes
// ----------------------------------------------------------------------------

Entity Painter::rect(float x, float y, float w, float h) {
    Entity e = spawn(*reg_, "Rectangle");
    auto& c = e.add<rectangle_component>();
    c.x = x; c.y = y; c.width = w; c.height = h;
    applyStyle(c);
    return finish2D(e);
}

Entity Painter::circle(float x, float y, float r) {
    Entity e = spawn(*reg_, "Circle");
    auto& c = e.add<circle_component>();
    c.x = x; c.y = y; c.radius = r;
    applyStyle(c);
    return finish2D(e);
}

Entity Painter::ellipse(float x, float y, float w, float h) {
    Entity e = spawn(*reg_, "Ellipse");
    auto& c = e.add<ellipse_component>();
    c.x = x; c.y = y; c.radiusX = w * 0.5f; c.radiusY = h * 0.5f;
    applyStyle(c);
    return finish2D(e);
}

Entity Painter::line(glm::vec2 a, glm::vec2 b) {
    Entity e = spawn(*reg_, "Line");
    auto& c = e.add<line_component>(a, b);
    // A line is a stroke: use the stroke colour/width (fall back to fill colour
    // if only a fill was set, so a bare drawLine still shows something).
    c.color     = style_.stroked ? style_.stroke : style_.fill;
    c.lineWidth = style_.strokeWidth;
    return finish2D(e);
}

Entity Painter::triangle(glm::vec2 a, glm::vec2 b, glm::vec2 c) {
    Entity e = spawn(*reg_, "Triangle");
    auto& comp = e.add<triangle_component>(a, b, c);
    applyStyle(comp);
    return finish2D(e);
}

Entity Painter::path(const ofPath& p) {
    Entity e = spawn(*reg_, "Path");
    auto& c = e.add<path_component>();
    c.path = p;
    applyStyle(c);
    // Keep the embedded ofPath's own style in sync with the component fields.
    c.path.setFilled(c.filled);
    c.path.setFillColor(c.fillColor);
    c.path.setStrokeColor(c.strokeColor);
    c.path.setStrokeWidth(c.strokeWidth);
    return finish2D(e);
}

// ----------------------------------------------------------------------------
// Painter — 3D meshes
// ----------------------------------------------------------------------------

Entity Painter::finishMesh(Entity e, glm::vec3 pos) {
    // Place the node by the current matrix, offset by the requested position.
    const glm::mat4 world = matrix_ * glm::translate(glm::mat4(1.f), pos);
    glm::vec3 t, s; glm::quat r;
    decomposeTRS(world, t, r, s);
    e.get<node_component>().node.setPositionOrientationScale(t, r, s);
    return finishCommon(e);
}

Entity Painter::mesh(eMeshPrimitiveType type, const std::string& name, glm::vec3 pos) {
    Entity e = spawn(*reg_, name);
    e.add<node_component>(name);
    auto& m = e.add<mesh_component>();
    m.primitiveType = type;
    applyMeshStyleTo(m, style_);
    switch (type) {                       // sensible per-type default sizes
        case MESH_SPHERE:    m.radius = 55.f; break;
        case MESH_CONE:      m.radius = 45.f; m.height = 130.f; break;
        case MESH_CYLINDER:  m.radius = 40.f; m.height = 120.f; break;
        case MESH_ICOSPHERE: m.radius = 60.f; break;
        case MESH_PLANE:     m.width = m.height = 200.f; break;
        default:             m.width = m.height = m.depth = 100.f; break;
    }
    m.rebuild();
    return finishMesh(e, pos);
}

Entity Painter::box(const std::string& name, glm::vec3 pos, float size) {
    Entity e = spawn(*reg_, name);
    e.add<node_component>(name);
    auto& m = e.add<mesh_component>();
    m.primitiveType = MESH_BOX;
    applyMeshStyleTo(m, style_);
    m.width = m.height = m.depth = size;
    m.rebuild();
    return finishMesh(e, pos);
}

Entity Painter::sphere(const std::string& name, glm::vec3 pos, float radius) {
    Entity e = spawn(*reg_, name);
    e.add<node_component>(name);
    auto& m = e.add<mesh_component>();
    m.primitiveType = MESH_SPHERE;
    applyMeshStyleTo(m, style_);
    m.radius = radius;
    m.rebuild();
    return finishMesh(e, pos);
}

Entity Painter::cone(const std::string& name, glm::vec3 pos, float radius, float height) {
    Entity e = spawn(*reg_, name);
    e.add<node_component>(name);
    auto& m = e.add<mesh_component>();
    m.primitiveType = MESH_CONE;
    applyMeshStyleTo(m, style_);
    m.radius = radius; m.height = height;
    m.rebuild();
    return finishMesh(e, pos);
}

Entity Painter::cylinder(const std::string& name, glm::vec3 pos, float radius, float height) {
    Entity e = spawn(*reg_, name);
    e.add<node_component>(name);
    auto& m = e.add<mesh_component>();
    m.primitiveType = MESH_CYLINDER;
    applyMeshStyleTo(m, style_);
    m.radius = radius; m.height = height;
    m.rebuild();
    return finishMesh(e, pos);
}

Entity Painter::icosphere(const std::string& name, glm::vec3 pos, float radius) {
    Entity e = spawn(*reg_, name);
    e.add<node_component>(name);
    auto& m = e.add<mesh_component>();
    m.primitiveType = MESH_ICOSPHERE;
    applyMeshStyleTo(m, style_);
    m.radius = radius;
    m.rebuild();
    return finishMesh(e, pos);
}

Entity Painter::plane(const std::string& name, glm::vec3 pos, float w, float h) {
    Entity e = spawn(*reg_, name);
    e.add<node_component>(name);
    auto& m = e.add<mesh_component>();
    m.primitiveType = MESH_PLANE;
    applyMeshStyleTo(m, style_);
    m.width = w; m.height = h;
    m.rebuild();
    return finishMesh(e, pos);
}

// ----------------------------------------------------------------------------
// Way B — free immediate API over a single current Painter.
// ----------------------------------------------------------------------------

namespace {
std::unique_ptr<Painter> g_current;
}

void beginPainter(const Layer& layer)     { g_current = std::make_unique<Painter>(layer); }
void beginPainter(entt::registry& reg)    { g_current = std::make_unique<Painter>(reg); }
void endPainter()                         { g_current.reset(); }
bool hasCurrentPainter()                  { return g_current != nullptr; }

// Layer::begin()/end() declared in layer_helpers.h — defined here so the Layer
// handle stays free of any painter dependency.
void Layer::begin() { beginPainter(*this); }
void Layer::end()   { endPainter(); }

Painter& currentPainter() {
    if (!g_current) {
        ofLogError("ecs::Painter") << "currentPainter() called outside begin()/end()";
    }
    return *g_current;
}

void fill(const ofColor& c)            { if (g_current) g_current->fill(c); }
void noFill()                          { if (g_current) g_current->noFill(); }
void stroke(const ofColor& c, float w) { if (g_current) g_current->stroke(c, w); }
void noStroke()                        { if (g_current) g_current->noStroke(); }

void pushMatrix()                      { if (g_current) g_current->pushMatrix(); }
void popMatrix()                       { if (g_current) g_current->popMatrix(); }
void resetMatrix()                     { if (g_current) g_current->resetMatrix(); }
void translate(const glm::vec3& t)     { if (g_current) g_current->translate(t); }
void rotateDeg(float deg, const glm::vec3& axis) { if (g_current) g_current->rotateDeg(deg, axis); }
void scale(const glm::vec3& s)         { if (g_current) g_current->scale(s); }
void scale(float s)                    { if (g_current) g_current->scale(s); }

Entity drawRect(float x, float y, float w, float h)    { return g_current ? g_current->rect(x, y, w, h)    : Entity{}; }
Entity drawCircle(float x, float y, float r)           { return g_current ? g_current->circle(x, y, r)     : Entity{}; }
Entity drawEllipse(float x, float y, float w, float h) { return g_current ? g_current->ellipse(x, y, w, h) : Entity{}; }
Entity drawLine(glm::vec2 a, glm::vec2 b)              { return g_current ? g_current->line(a, b)          : Entity{}; }
Entity drawTriangle(glm::vec2 a, glm::vec2 b, glm::vec2 c) { return g_current ? g_current->triangle(a, b, c) : Entity{}; }
Entity drawPath(const ofPath& p)                       { return g_current ? g_current->path(p)             : Entity{}; }

Entity drawBox(const std::string& name, glm::vec3 pos, float size)                 { return g_current ? g_current->box(name, pos, size)            : Entity{}; }
Entity drawSphere(const std::string& name, glm::vec3 pos, float radius)            { return g_current ? g_current->sphere(name, pos, radius)        : Entity{}; }
Entity drawCone(const std::string& name, glm::vec3 pos, float radius, float height){ return g_current ? g_current->cone(name, pos, radius, height)   : Entity{}; }
Entity drawCylinder(const std::string& name, glm::vec3 pos, float radius, float height){ return g_current ? g_current->cylinder(name, pos, radius, height) : Entity{}; }
Entity drawIcosphere(const std::string& name, glm::vec3 pos, float radius)         { return g_current ? g_current->icosphere(name, pos, radius)     : Entity{}; }
Entity drawPlane(const std::string& name, glm::vec3 pos, float w, float h)         { return g_current ? g_current->plane(name, pos, w, h)           : Entity{}; }

} // namespace ecs
