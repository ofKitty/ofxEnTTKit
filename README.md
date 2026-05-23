ofxEnTTKit
==========

![thumb](ofxaddons_thumbnail.png)

ECS toolkit for openFrameworks built on top of [EnTT](https://github.com/skypjack/entt) ([ofxEnTT](https://github.com/ofrasp/ofxEnTT)). `ofxEnTTKit` provides an opinionated set of components and systems for typical creative-coding needs (scene graph, 3D rendering, 2D graphics, animation, hardware I/O, LEDs, state/modulators, etc.) together with a lightweight property-reflection layer suitable for ImGui-based inspectors.

The kit aims to capture the common openFrameworks objects — `nodes, cameras, meshes, materials, lights, shaders, textures, FBOs, images, video, audio, fonts, paths, polylines, serial I/O, and related drawing helpers` — as ECS-friendly components.

All components live in the `ecs::` namespace and are plain data structs, so they can be picked up by whichever GUI/serialization layer your app uses.

## Dependencies

- [ofxEnTT](https://github.com/ofrasp/ofxEnTT) — provides the EnTT header

## Setup

Include the umbrella header from your app — it pulls in `<entt.hpp>` plus all bundled components and systems:

```cpp
#include "ofxEnTTKit.h"
```

Or, for faster compiles, include only the component/system headers you need from `src/components/` and `src/systems/`. If you don't want the ECS layer at all and just need EnTT, depend on `ofxEnTT` directly and `#include <entt.hpp>`.

### “Add Component” menu data (editor shells)

The picker registry lives in **ofxEnTTKit** — see [`docs/component-registry.md`](docs/component-registry.md). This is **not** EnTT type registration; it only produces **picker metadata** (display name, category, `has` / `add` / `remove` hooks) for shipped and addon `ecs::*` components.

- **Addons** extend the registry via `ecs::registerComponent<T>(...)`.
- **ofxKit** calls `ecs::finalizeComponentMenu()` on attach and reads `ecs::componentMenuEntries()` for the Properties picker.

ImGui property drawing remains type-based in **ofxEnTTInspector**; the picker registry only drives **add/remove** from an editor menu.

## What's inside

See [`COMPONENTS_REFERENCE.md`](COMPONENTS_REFERENCE.md) for the full component catalogue and [`SYSTEMS_REFERENCE.md`](SYSTEMS_REFERENCE.md) for the systems reference (purpose, components consumed, public API, frame loop ordering).

Quick overview:

| area | headers |
|------|---------|
| base (nodes, cameras, meshes, audio, video, …) | `components/base_components.h` |
| 3D rendering (lights, materials, shaders, …)   | `components/rendering_components.h` |
| 2D graphics (paths, shapes, sprites, text)     | `components/graphics2d_components.h`, `components/shape2d_component.h` |
| animation (tween, particles, postfx)           | `components/animation_components.h`, `components/eased_pulse_component.h`, `components/easing.h` |
| modulators / state / swatches                  | `components/modulator_component.h`, `components/state_components.h`, `components/swatch_components.h` |
| LED / UV mapping                               | `components/led_components.h` |
| hardware I/O (serial, OSC)                     | `components/hardware_components.h` |
| audio/MIDI sources                             | `components/audio_source_component.h`, `components/midi_source_component.h` |
| filters / generators / effects / canvas        | `components/filter_components.h`, `components/generator_components.h`, `components/draw_filter_components.h`, `components/canvas_effects_component.h` |
| utility (grid, gizmo, AABB, rigidbody, mask)   | `components/utility_components.h` |
| systems                                        | `systems/*.h` (via `systems/systems.h`) |
| editor **Add Component** registry | `component_editor_registration.h` — see [`docs/component-registry.md`](docs/component-registry.md) |

> Add [ofxEnTTInspector](https://github.com/ofKitty/ofxEnTTInspector) for a unified reflection and serialization foundation.

## License

MIT — see `license.md`.
