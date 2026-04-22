# ofxEnTTKit — Components & Systems Reference

This is the authoritative index of every component and system shipped with
`ofxEnTTKit`. All types live in the `ecs::` namespace. Components are pure
data structs; systems contain the logic that operates on them.

## Conventions

- **Components** — POD-ish structs named `*_component` (data only)
- **Markers** — empty tag structs used by views/queries (`drawable`, `updateable`, …)
- **Helpers** — supporting structs nested inside a component file (`LEDPixel`, `SwatchColor`, …)
- **Systems** — classes deriving from `ISystem` with `update()` / `draw()` / `setup()` / `cleanup()`

To use the whole kit: `#include "ofxEnTTKit.h"`. To minimize compile time,
include only the specific `components/*.h` / `systems/*.h` headers you need.

---

## At a glance

| category | file | # components |
|---|---|---|
| Base (transform, media, canvas, meta) | `components/base_components.h` | 20 + 5 markers |
| 3D Rendering | `components/rendering_components.h` | 15 |
| 2D Graphics | `components/graphics2d_components.h` | 24 |
| Shape2D | `components/shape2d_component.h` | 1 |
| Animation | `components/animation_components.h` | 3 |
| Eased pulse | `components/eased_pulse_component.h` | 1 |
| Easing (enum + helpers) | `components/easing.h` | — |
| Modulators | `components/modulator_component.h` | 2 |
| State / morph / timeline | `components/state_components.h` | 4 |
| Swatches / gradients | `components/swatch_components.h` | 2 |
| Hardware I/O | `components/hardware_components.h` | 2 |
| Audio source | `components/audio_source_component.h` | 1 |
| MIDI source | `components/midi_source_component.h` | 1 |
| Canvas effects | `components/canvas_effects_component.h` | 1 |
| Filters (sampling) | `components/filter_components.h` | 12 |
| Draw filters (geometry) | `components/draw_filter_components.h` | 6 |
| Generators | `components/generator_components.h` | 5 |
| LED / UV mapping | `components/led_components.h` | 2 |
| Utility (debug, physics, mask) | `components/utility_components.h` | 5 |

≈ **110 components** total across 20 headers.

---

## Components

### Base — `components/base_components.h`

**Markers**

| marker | purpose |
|---|---|
| `app_settings` | Tag root entity that carries app-level state |
| `drawable` | Entity participates in draw systems |
| `updateable` | Entity participates in update systems |
| `lightable` | Entity receives scene lighting (`bool enableLighting`) |
| `scene_component` | Tag for scene-root entities |

**Transform & hierarchy**

| component | purpose |
|---|---|
| `node_component` | Transform via `myNode` (ofNode + dirty/change events); stable-address `cachedPosition`/`cachedScale` for reflection |
| `parent_component` | Parent entity + ordered children list; destroy handler cleans up hierarchy |
| `camera_component` | `ofCamera` + active flag + 3 view presets (2D / Ortho / Perspective) |

**Media**

| component | purpose |
|---|---|
| `audio_component` | `ofSoundPlayer` backed by file path |
| `image_component` | `ofImage` loaded from disk |
| `video_component` | `ofVideoPlayer` with looping |
| `model_component` | `ofxAssimpModelLoader` + optional override material |
| `mesh_component` | `ofMesh` + procedural primitive settings (`rebuild()`) |
| `text_component` | `ofTrueTypeFont` + text string |
| `soundStream_component` | Wraps `ofSoundStream` with its settings |
| `soundStream_settings_component` | Standalone `ofSoundStreamSettings` holder |
| `audio_settings_component` | Per-entity volume / pan / pause |

**Canvas**

| component | purpose |
|---|---|
| `fbo_component` | `ofFbo` + clear color, size, format, dirty flag, `reallocate()` |
| `fbo_reference_component` | Draws another FBO entity (instance/reference; validates at runtime) |

**Meta**

| component | purpose |
|---|---|
| `tag_component` | Free-form `std::string` tag |
| `render_component` | Render `order`, `enableLighting`, `visible` flags |
| `selectable_component` | Selection state |
| `fresh_component` | "Dirty tracker" with frame-accurate last-clean timestamp |
| `filepath_component` | `std::filesystem::path` with accessors (name, ext, exists…) |
| `resource_component` | Type + status (`ERS_OK` / `ERS_MISSING`), auto-checks via filepath |

**Helpers (non-component)**

- `CameraViewPreset` — serializable camera state (pos/orient/scale/near/far + NaN validation)

**Free functions**

- `onParentDestroy`, `onCameraDestroy` — registry destroy callbacks
- `getActiveCamera`, `getActiveCameraEntity`, `setActiveCamera`
- `ecs::g_idCounter` — monotonic ID used by `node_component`

---

### 3D Rendering — `components/rendering_components.h`

| component | purpose |
|---|---|
| `light_component` | `ofLight` wrapper: POINT / DIRECTIONAL / SPOT / AREA, attenuation, shadow/debug flags |
| `material_component` | `ofMaterial` with Phong + PBR (metallic, roughness, ao); `begin()`/`end()` |
| `shader_component` | `ofShader` with vert/frag/geom paths, uniform maps, hot-reload (`checkForReload()`) |
| `primitive_component` | `of3dPrimitive` (BOX / SPHERE / CYLINDER / CONE / PLANE / ICOSPHERE) |
| `texture_component` | `ofTexture` + UV transform |
| `cubemap_component` | Environment map (6 faces) |
| `skybox_component` | Skybox drawn with a cubemap |
| `billboard_component` | Camera-facing quad |
| `trail_component` | Ribbon / trail renderer |
| `instanced_mesh_component` | GPU-instanced mesh drawing |
| `tube_component` | Tube extruded along a path |
| `surface_component` | Procedurally-generated surface mesh |
| `outline_component` | Object silhouette/outline pass |
| `shadow_component` | Drop shadow renderer helper |
| `glow_component` | Radial glow helper |

---

### 2D Graphics — `components/graphics2d_components.h`

**Paths & primitives**

| component | purpose |
|---|---|
| `path_component` | `ofPath` vector graphics |
| `polyline_component` | `ofPolyline` with smoothing/resampling |
| `rectangle_component` | Rectangle with optional rounded corners |
| `circle_component` | Circle |
| `ellipse_component` | Axis-aligned ellipse |
| `line_component` | Line with optional arrow caps |
| `triangle_component` | Triangle |
| `polygon_component` | Arbitrary N-gon |
| `regular_polygon_component` | Regular N-sided polygon |
| `star_component` | Star shape |
| `arc_component` | Arc / pie slice |
| `bezier_curve_component` | Cubic Bezier |
| `spline_component` | Catmull-Rom spline |
| `ring_component` | Ring / annulus |
| `cross_component` | Cross / plus |
| `heart_component` | Heart shape |

**Sprites, text, gradients**

| component | purpose |
|---|---|
| `sprite_component` | Textured quad with anchor, tint, flip |
| `text_2d_component` | 2D text with alignment |
| `gradient_component` | Linear / radial gradient |

**Helpers / HUD**

| component | purpose |
|---|---|
| `grid_component` | 2D grid overlay |
| `progress_bar_component` | Progress bar widget |
| `arrow_component` | Directional arrow |
| `soft_mask_component` | Soft-edged rectangular mask |

**Sacred geometry**

| component | purpose |
|---|---|
| `vesica_piscis_component` | Vesica Piscis |
| `flower_of_life_component` | Flower of Life |
| `metatrons_cube_component` | Metatron's Cube |

---

### Shape2D — `components/shape2d_component.h`

| component | purpose |
|---|---|
| `shape2d_component` | Generic unified 2D shape (type enum + parameters) as an alternative to dedicated shape components |

---

### Animation — `components/animation_components.h`

| component | purpose |
|---|---|
| `tween_component` | Value tween with 27 easing functions, loop/pingpong, `onUpdate`/`onComplete` callbacks |
| `particle_emitter_component` | CPU particle system (emit, life, gravity, damping, color-over-life) |
| `postfx_component` | Post-processing chain (BLOOM, BLUR, CHROMATIC_ABERRATION, VIGNETTE, GRAIN, SCANLINES, CRT, GLITCH, COLOR_GRADING, FXAA, DOF, SSAO) |

---

### Eased Pulse — `components/eased_pulse_component.h`

| component | purpose |
|---|---|
| `eased_pulse_component` | One-shot/loop pulse driven by an easing curve (great for flashes, bursts, retriggering) |

### Easing — `components/easing.h`

Not a component. Ships the `EasingType` enum (27 curves) and `easing::apply(type, t)`
— reused by `tween_component`, `state_morph_component`, `state_timeline_component`
and `eased_pulse_component`.

---

### Modulators — `components/modulator_component.h`

| component | purpose |
|---|---|
| `modulator_component` | LFO generator (SIN / SQUARE / TRIANGLE / SAW / NOISE) with frequency, phase, amplitude |
| `mod_binding_component` | Binds a modulator output to a `float*` target with depth, min/max range, additive mode |

---

### State / morph / timeline — `components/state_components.h`

| component | purpose |
|---|---|
| `state_preset_component` | Named snapshot of entity properties (`entity_snapshot[]`) |
| `state_morph_component` | Animate from one preset index to another over `duration` with easing |
| `state_timeline_component` | Keyframed timeline of preset indices with BPM sync, loop, playback speed |
| `state_library_component` | Collection of presets with add/remove/duplicate/select |

Helpers: `property_snapshot`, `entity_snapshot`, `timeline_keyframe`.

---

### Swatches / gradients — `components/swatch_components.h`

| component | purpose |
|---|---|
| `swatch_library_component` | Named palette library (add/find/remove swatches, color pick) |
| `color_gradient_component` | Multi-stop gradient with interpolation mode and sampling |

Helpers: `SwatchColor`, `GradientStop`.

---

### Hardware I/O — `components/hardware_components.h`

| component | purpose |
|---|---|
| `serial_component` | `ofSerial` wrapper — connect, send/receive, auto-reconnect, `listDevices()` |
| `osc_component` | OSC sender/receiver with host/port settings |

---

### Audio source — `components/audio_source_component.h`

| component | purpose |
|---|---|
| `audio_source_component` | Audio analysis source: live input or soundplayer → FFT bands, onset/beat detection |

Helpers: `FFTBandRange`, `beat_detector`.

---

### MIDI source — `components/midi_source_component.h`

| component | purpose |
|---|---|
| `midi_source_component` | MIDI input wrapper with note state tracking and CC → float mappings |

Helpers: `midi_note_state`, `midi_cc_mapping`.

---

### Canvas effects — `components/canvas_effects_component.h`

| component | purpose |
|---|---|
| `canvas_effects_component` | Per-canvas full-screen effects applied to an FBO (feedback, trails, etc.) |

---

### Filters (sampling) — `components/filter_components.h`

All inherit `filter_base` (order + enabled).

| component | purpose |
|---|---|
| `tint_filter_component` | Multiplicative color tint |
| `invert_filter_component` | Invert colors |
| `mirror_filter_component` | Mirror horizontally/vertically/quadrants |
| `color_adjust_component` | Hue / saturation / brightness / contrast |
| `blur_filter_component` | Gaussian blur |
| `dither_filter_component` | Dithering (ordered / error-diffusion) |
| `rotate_filter_component` | Rotate image by angle |
| `threshold_filter_component` | Binary threshold |
| `posterize_filter_component` | Color quantization |
| `noise_filter_component` | Add noise |
| `vignette_filter_component` | Radial darkening |
| `chromatic_aberration_component` | RGB channel offset |

---

### Draw filters (geometry) — `components/draw_filter_components.h`

| component | purpose |
|---|---|
| `pixelate_filter_component` | Pixelate an FBO/texture |
| `mesh_filter_component` | Render via a displaced mesh grid |
| `rings_filter_component` | Concentric rings sampling |
| `noise_displacement_component` | Noise-driven UV displacement |
| `line_scan_filter_component` | Line-scan / slit-scan effect |
| `ascii_filter_component` | ASCII-art rendering |

---

### Generators — `components/generator_components.h`

| component | purpose |
|---|---|
| `gradient_generator_component` | Procedural linear/radial gradient to FBO |
| `dots_generator_component` | Dot grid pattern |
| `stripes_generator_component` | Stripe pattern |
| `checkerboard_generator_component` | Checkerboard pattern |
| `noise_generator_component` | Perlin / simplex noise field |

Helper: `gradient_stop`.

---

### LED / UV mapping — `components/led_components.h`

| component | purpose |
|---|---|
| `uv_component` | LED UV map: normalized 0-1 coords per LED with canvas dimensions, JSON load/save |
| `uv_sample_component` | Composes with `fbo_component` to sample LED colors from a canvas |

Helper: `LEDPixel`.

---

### Utility — `components/utility_components.h`

| component | purpose |
|---|---|
| `grid_helper_component` | 3D grid visualization with RGB axes |
| `gizmo_component` | Transform manipulation gizmo (translate / rotate / scale) |
| `bounding_box_component` | AABB for collision / selection |
| `mask_component` | Stencil-based masking |
| `rigidbody_component` | Simple rigidbody (velocity, acceleration, drag, gravity) — no collision |

---

## Systems

Systems live in `src/systems/` and all derive from `ecs::ISystem`. Pull them in
together via `#include "systems/systems.h"` or individually.

| system | file | update | draw | purpose |
|---|---|:---:|:---:|---|
| `DestroySystem`           | `destroy_system.h`           |   | | Wires `on_destroy` handlers for `parent_component` (hierarchy cleanup) & `camera_component` (active-camera switching) |
| `MediaRenderSystem`       | `media_render_system.h`      | ✓ | ✓ | Draws `image_component`, `video_component`, `fbo_component`; updates videos |
| `MeshRenderSystem`        | `mesh_render_system.h`       |   | ✓ | Draws `mesh_component`, `model_component`, `primitive_component`, `billboard_component`, `instanced_mesh_component`, `tube_component`, `surface_component`; mesh primitive helpers |
| `Graphics2DRender`        | `graphics2d_system.h`        |   | ✓ | Draws every 2D graphics / shape component; pluggable `IShapeRenderer`; optional `BatchRenderer` batching; vector-export hook |
| `AnimationSystem`         | `animation_system.h`         | ✓ | ✓ | Updates tweens, particle emitters, postFX; draws particles |
| `ModulatorSystem`         | `modulator_system.h`         | ✓ |   | Updates modulators, applies outputs to their bindings |
| `PhysicsSystem`           | `physics_system.h`           | ✓ |   | Rigidbody integration with gravity & drag (no collision) |
| `UtilityRenderSystem`     | `utility_render_system.h`    |   | ✓ | Draws `grid_helper_component` and `bounding_box_component` |
| `GizmoSystem`             | `gizmo_system.h`             |   | ✓ | Draws translate / rotate / scale gizmos |
| `HardwareIOSystem`        | `hardware_io_system.h`       | ✓ |   | Drives `serial_component` and `osc_component` |
| `TrailSystem`             | `trail_system.h`             | ✓ | ✓ | Updates trail points; draws ribbons (needs camera) |
| `EffectsRenderSystem`     | `effects_render_system.h`    |   |   | Static helpers for `shadow_component`, `glow_component`, `outline_component` |
| `LEDSystem`               | `led_system.h`               | ✓ | ✓ | Samples UV maps from textures/FBOs/pixels; preview draw; output buffers |
| `UVRenderSystem`          | `uv_render_system.h`         |   | ✓ | Draws LED positions with sampled colors (optionally using a drawable entity as the per-pixel shape) |

Plus supporting classes:

| class | file | purpose |
|---|---|---|
| `SystemManager` | `base_system.h` | Owns systems, dispatches `setup` / `update` / `draw` / `cleanup`, supports `getSystem<T>()` |
| `ISystem`       | `base_system.h` | Base system interface |
| `BatchRenderer` | `batch_renderer.h` | Tessellates strokes + collects triangles for batched 2D drawing |
| `IShapeRenderer`| `IShapeRenderer.h` | Abstract interface for pluggable stroke/fill rendering (e.g. backed by `ofxVaseRenderer`) |

---

## Patterns

### Instantiation

```cpp
auto e = registry.create();
auto& node = registry.emplace<ecs::node_component>(e);
auto& mesh = registry.emplace<ecs::mesh_component>(e);
mesh.primitiveType = ecs::MESH_SPHERE;
mesh.radius = 50.0f;
mesh.rebuild();
```

### Iterating

```cpp
auto view = registry.view<ecs::node_component, ecs::mesh_component>();
for (auto e : view) {
    auto& [node, mesh] = view.get<ecs::node_component, ecs::mesh_component>(e);
    // ...
}
```

### Running systems

```cpp
ecs::SystemManager systems;
systems.addSystem<ecs::DestroySystem>();
systems.addSystem<ecs::MeshRenderSystem>();
systems.addSystem<ecs::AnimationSystem>();
systems.setup(registry);

// per frame
systems.update(registry, dt);
systems.draw(registry);
```

### Binding a modulator to a value

```cpp
auto e = registry.create();
auto& mod = registry.emplace<ecs::modulator_component>(e);
mod.waveType  = ecs::MOD_SIN;
mod.frequency = 0.5f;

registry.emplace<ecs::mod_binding_component>(e, &myNode.position.y, 1.0f, -50.f, 50.f, true);
```

### Saving / loading an LED UV map

```cpp
auto e = registry.create();
auto& uv = registry.emplace<ecs::uv_component>(e);
uv.loadFromJSON("mapping_uv.json");
// later
uv.saveToJSON("esp32/mapping_uv.json");
```

---

## Notes

- Reflection / GUI registration (`registerProperties()`) lives **next to** each
  component but depends on whichever inspector layer your app provides
  (ImGui / ofParameter / custom). `ofxEnTTKit` itself does not ship an
  inspector.
- Several `*_component` constructors do real work (load audio/image, allocate
  FBO). Call `emplace<>()` on the main thread where OpenGL calls are valid.
- `rendering_components.h` forward-declares `ofLight` to keep the header
  lightweight; include `ofMain.h` / `ofLight.h` in your .cpp to use it.
