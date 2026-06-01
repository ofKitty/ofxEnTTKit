#include "component_editor_registration.h"

#include "ofxEnTTKit.h"
#include "ofxEnTTKit_all_components.h"

#include <algorithm>
#include <vector>

namespace ecs {
namespace {

bool& componentMenuFinalized()
{
    static bool finalized = false;
    return finalized;
}

std::vector<ComponentMenuEntry>& componentMenuAll()
{
    static std::vector<ComponentMenuEntry> entries;
    return entries;
}

std::vector<ComponentMenuEntry>& componentMenuExtensions()
{
    static std::vector<ComponentMenuEntry> entries;
    return entries;
}

template<typename T>
void appendSimple(std::vector<ComponentMenuEntry>& out,
                  const char*                     name,
                  const char*                     category)
{
    ComponentMenuEntry d;
    d.name     = name;
    d.category = category;
    d.has      = [](entt::registry& r, entt::entity e) { return r.all_of<T>(e); };
    d.add      = [](entt::registry& r, entt::entity e) {
        if (!r.all_of<T>(e)) r.emplace<T>(e);
    };
    d.remove = [](entt::registry& r, entt::entity e) { r.remove<T>(e); };
    out.push_back(std::move(d));
}

void registerBuiltInComponents(std::vector<ComponentMenuEntry>& out)
{
    appendSimple<node_component>(out, "Node", "Transform");
    appendSimple<tag_component>(out, "Tag", "Transform");
    appendSimple<code_snippet_component>(out, "Code Snippet", "Transform");
    appendSimple<selectable_component>(out, "Selectable", "Transform");
    appendSimple<filepath_component>(out, "File Path", "Transform");

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
        out.push_back(std::move(d));
    }
    appendSimple<render_component>(out, "Render", "3D");
    appendSimple<light_component>(out, "Light", "3D");
    appendSimple<material_component>(out, "Material", "3D");
    appendSimple<shader_component>(out, "Shader", "3D");
    appendSimple<primitive_component>(out, "Primitive", "3D");
    appendSimple<billboard_component>(out, "Billboard", "3D");
    appendSimple<tube_component>(out, "Tube", "3D");
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
        out.push_back(std::move(d));
    }
    appendSimple<instanced_mesh_component>(out, "Instanced Mesh", "3D");
    appendSimple<cubemap_component>(out, "Cubemap", "3D");
    appendSimple<texture_component>(out, "Texture", "3D");
    appendSimple<outline_component>(out, "Outline", "3D");
    appendSimple<glow_component>(out, "Glow", "3D");
    appendSimple<shadow_component>(out, "Shadow", "3D");
    appendSimple<camera_component>(out, "Camera", "Camera");
    appendSimple<skybox_component>(out, "Skybox", "3D");
    appendSimple<postfx_component>(out, "Post FX", "Rendering");
    appendSimple<trail_component>(out, "Trail", "3D");

    appendSimple<shape2d_component>(out, "Shape 2D", "2D");
    appendSimple<circle_component>(out, "Circle", "2D");
    appendSimple<rectangle_component>(out, "Rectangle", "2D");
    appendSimple<ellipse_component>(out, "Ellipse", "2D");
    appendSimple<line_component>(out, "Line", "2D");
    appendSimple<triangle_component>(out, "Triangle", "2D");
    appendSimple<polygon_component>(out, "Polygon", "2D");
    appendSimple<arc_component>(out, "Arc", "2D");
    appendSimple<bezier_curve_component>(out, "Bezier", "2D");
    appendSimple<spline_component>(out, "Spline", "2D");
    appendSimple<path_component>(out, "Path", "2D");
    appendSimple<polyline_component>(out, "Polyline", "2D");
    appendSimple<text_2d_component>(out, "Text", "2D");
    appendSimple<sprite_component>(out, "Sprite", "2D");
    appendSimple<gradient_component>(out, "Gradient", "2D");

    appendSimple<canvas_effects_component>(out, "Canvas FX", "Rendering");

    appendSimple<image_component>(out, "Image", "Media");
    appendSimple<video_component>(out, "Video", "Media");
    appendSimple<fbo_component>(out, "FBO", "Media");
    appendSimple<fbo_reference_component>(out, "FBO Reference", "Media");
    appendSimple<audio_component>(out, "Audio", "Media");

    appendSimple<rigidbody_component>(out, "Rigidbody", "Physics");

    appendSimple<tween_component>(out, "Tween", "Animation");
    appendSimple<particle_emitter_component>(out, "Particles", "Animation");
    appendSimple<modulator_component>(out, "Modulator", "Modulation");
    appendSimple<mod_binding_component>(out, "Mod Binding", "Modulation");
    appendSimple<transport_control_component>(out, "Transport", "Music");
    appendSimple<clock_component>(out, "Clock", "Music");
    appendSimple<sequencer_component>(out, "Sequencer", "Music");
    appendSimple<pattern_component>(out, "Pattern", "Music");
    appendSimple<midi_output_component>(out, "MIDI Output", "Music");
    appendSimple<trigger_lane_component>(out, "Trigger Lane", "Music");
    appendSimple<trigger_pattern_component>(out, "Trigger Pattern", "Music");
    appendSimple<trigger_pattern_data_component>(out, "Trigger Pattern Data", "Music");
    appendSimple<trigger_sequencer_component>(out, "Trigger Sequencer", "Music");

    appendSimple<color_band_component>(out, "Color Band", "Color");

    appendSimple<serial_component>(out, "Serial", "Hardware");
    appendSimple<osc_component>(out, "OSC", "Hardware");
    appendSimple<audio_source_component>(out, "Audio Source", "Hardware");
    appendSimple<midi_source_component>(out, "MIDI", "Hardware");
    appendSimple<mmwave_c4001_component>(out, "mmWave C4001", "Hardware");
    appendSimple<gpio_component>(out, "GPIO Trigger", "Hardware");

    appendSimple<keyboard_input_component>(out, "Keyboard Input", "Input");

    appendSimple<uv_component>(out, "UV LED Map", "LED");
    appendSimple<uv_sample_component>(out, "UV Sample", "LED");
}

} // namespace

void registerComponentMenuEntry(ComponentMenuEntry entry)
{
    if (componentMenuFinalized()) {
        componentMenuAll().push_back(std::move(entry));
        return;
    }
    componentMenuExtensions().push_back(std::move(entry));
}

void finalizeComponentMenu()
{
    if (componentMenuFinalized()) return;
    componentMenuFinalized() = true;

    auto& all = componentMenuAll();
    registerBuiltInComponents(all);

    for (auto& entry : componentMenuExtensions()) {
        all.push_back(std::move(entry));
    }
    componentMenuExtensions().clear();
}

const std::vector<ComponentMenuEntry>& componentMenuEntries()
{
    if (!componentMenuFinalized()) {
        finalizeComponentMenu();
    }
    return componentMenuAll();
}

std::vector<std::string> componentMenuCategories()
{
    const auto& entries = componentMenuEntries();
    std::vector<std::string> cats;
    cats.reserve(entries.size());
    for (const auto& entry : entries) {
        if (std::find(cats.begin(), cats.end(), entry.category) == cats.end()) {
            cats.push_back(entry.category);
        }
    }
    return cats;
}

} // namespace ecs
