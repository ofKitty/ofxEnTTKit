#include "component_editor_registration.h"

#include "ofxEnTTKit.h"

namespace ecs {
namespace {

template<typename T>
void registerSimple(ComponentMenuCallback sink,
                    const char*          name,
                    const char*          category)
{
    if (!sink) return;

    ComponentMenuEntry d;
    d.name     = name;
    d.category = category;
    d.has      = [](entt::registry& r, entt::entity e) { return r.all_of<T>(e); };
    d.add      = [](entt::registry& r, entt::entity e) {
        if (!r.all_of<T>(e)) r.emplace<T>(e);
    };
    d.remove = [](entt::registry& r, entt::entity e) { r.remove<T>(e); };
    sink(d);
}

} // namespace

void registerKitComponentMenu(ComponentMenuCallback sink)
{
    if (!sink) return;

    // ── Transform ────────────────────────────────────────────────────────────
    registerSimple<node_component>(sink, "Node", "Transform");
    registerSimple<tag_component>(sink, "Tag", "Transform");
    registerSimple<selectable_component>(sink, "Selectable", "Transform");
    registerSimple<filepath_component>(sink, "File Path", "Transform");

    // ── 3D ───────────────────────────────────────────────────────────────────
    {
        ComponentMenuEntry d;
        d.name     = "Mesh";
        d.category = "3D";
        d.has      = [](entt::registry& r, entt::entity e) {
            return r.all_of<mesh_component>(e);
        };
        d.add = [](entt::registry& r, entt::entity e) {
            auto& m = r.emplace_or_replace<mesh_component>(e);
            m.primitiveType = MESH_BOX;
            m.rebuild();
        };
        d.remove = [](entt::registry& r, entt::entity e) {
            r.remove<mesh_component>(e);
        };
        sink(d);
    }
    registerSimple<render_component>(sink, "Render", "3D");
    registerSimple<light_component>(sink, "Light", "3D");
    registerSimple<material_component>(sink, "Material", "3D");
    registerSimple<shader_component>(sink, "Shader", "3D");
    registerSimple<primitive_component>(sink, "Primitive", "3D");
    registerSimple<billboard_component>(sink, "Billboard", "3D");
    registerSimple<trail_component>(sink, "Trail", "3D");
    registerSimple<tube_component>(sink, "Tube", "3D");
    {
        ComponentMenuEntry d;
        d.name     = "Surface";
        d.category = "3D";
        d.has      = [](entt::registry& r, entt::entity e) {
            return r.all_of<surface_component>(e);
        };
        d.add = [](entt::registry& r, entt::entity e) {
            auto& s = r.emplace_or_replace<surface_component>(e);
            s.generateWave();
            s.rebuild();
        };
        d.remove = [](entt::registry& r, entt::entity e) {
            r.remove<surface_component>(e);
        };
        sink(d);
    }
    registerSimple<instanced_mesh_component>(sink, "Instanced Mesh", "3D");
    registerSimple<cubemap_component>(sink, "Cubemap", "3D");
    registerSimple<texture_component>(sink, "Texture", "3D");
    registerSimple<outline_component>(sink, "Outline", "3D");
    registerSimple<glow_component>(sink, "Glow", "3D");
    registerSimple<shadow_component>(sink, "Shadow", "3D");

    // ── 2D ───────────────────────────────────────────────────────────────────
    registerSimple<shape2d_component>(sink, "Shape 2D", "2D");
    registerSimple<circle_component>(sink, "Circle", "2D");
    registerSimple<rectangle_component>(sink, "Rectangle", "2D");
    registerSimple<ellipse_component>(sink, "Ellipse", "2D");
    registerSimple<line_component>(sink, "Line", "2D");
    registerSimple<triangle_component>(sink, "Triangle", "2D");
    registerSimple<polygon_component>(sink, "Polygon", "2D");
    registerSimple<arc_component>(sink, "Arc", "2D");
    registerSimple<bezier_curve_component>(sink, "Bezier", "2D");
    registerSimple<spline_component>(sink, "Spline", "2D");
    registerSimple<path_component>(sink, "Path", "2D");
    registerSimple<polyline_component>(sink, "Polyline", "2D");
    registerSimple<text_2d_component>(sink, "Text", "2D");
    registerSimple<sprite_component>(sink, "Sprite", "2D");
    registerSimple<gradient_component>(sink, "Gradient", "2D");

    // ── Rendering ─────────────────────────────────────────────────────────────
    registerSimple<postfx_component>(sink, "Post FX", "Rendering");
    registerSimple<canvas_effects_component>(sink, "Canvas FX", "Rendering");

    // ── Media ────────────────────────────────────────────────────────────────
    registerSimple<image_component>(sink, "Image", "Media");
    registerSimple<video_component>(sink, "Video", "Media");
    registerSimple<fbo_component>(sink, "FBO", "Media");
    registerSimple<fbo_reference_component>(sink, "FBO Reference", "Media");

    // ── Camera ─────────────────────────────────────────────────────────────────
    registerSimple<camera_component>(sink, "Camera", "Camera");

    // ── Scene ─────────────────────────────────────────────────────────────────
    registerSimple<skybox_component>(sink, "Skybox", "3D");

    // ── Physics ───────────────────────────────────────────────────────────────
    registerSimple<rigidbody_component>(sink, "Rigidbody", "Physics");

    // ── Audio (media) ───────────────────────────────────────────────────────────
    registerSimple<audio_component>(sink, "Audio", "Media");

    // ── Animation ─────────────────────────────────────────────────────────────
    registerSimple<tween_component>(sink, "Tween", "Animation");
    registerSimple<particle_emitter_component>(sink, "Particles", "Animation");

    // ── Modulation ────────────────────────────────────────────────────────────
    registerSimple<modulator_component>(sink, "Modulator", "Modulation");
    registerSimple<mod_binding_component>(sink, "Mod Binding", "Modulation");

    // ── Color ─────────────────────────────────────────────────────────────────
    registerSimple<swatch_library_component>(sink, "Color Swatches", "Color");
    registerSimple<color_gradient_component>(sink, "Color Gradient", "Color");

    // ── Hardware ──────────────────────────────────────────────────────────────
    registerSimple<serial_component>(sink, "Serial", "Hardware");
    registerSimple<osc_component>(sink, "OSC", "Hardware");
    registerSimple<audio_source_component>(sink, "Audio Source", "Hardware");
    registerSimple<midi_source_component>(sink, "MIDI", "Hardware");
}

} // namespace ecs
