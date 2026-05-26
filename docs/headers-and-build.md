# Headers

ofxEnTTKit splits **core ECS** from **optional kit pieces** so apps can include only what they use.

## Include headers

| Header | What it pulls in |
|--------|------------------|
| `ofxEnTTKit.h` | EnTT, `entity_utils`, **core** components (`hierarchy`, `base`, `layer`, `state`), `ofxNode`, **core** systems (`destroy`, `layer`, `TransformSystem`) |
| `ofxEnTTKit_all_components.h` | All optional headers under `src/components/` |
| `ofxEnTTKit_all_systems.h` | All optional headers under `src/systems/` (excluding core systems already in `ofxEnTTKit.h`) |
| `ofxEnTTKit_all.h` | `ofxEnTTKit.h` + both `_all_*` umbrellas + `component_editor_registration.h` |

**Typical choices**

- **Kit / editor apps** (ofxKit, ofxBapp, inspectors): `#include "ofxEnTTKit_all.h"`
- **Minimal or headless apps** (e.g. ofxESP32): `#include "ofxEnTTKit.h"` plus any specific `components/` or `systems/` headers you need
- **Fine-grained**: pick individual files under `src/components/` and `src/systems/`

The addon builds all `src/**/*.cpp` by default. Include the headers for the types you use.

## Related docs

- [component-registry.md](component-registry.md) — “Add Component” picker
- [../COMPONENTS_REFERENCE.md](../COMPONENTS_REFERENCE.md) — component catalogue
- [../SYSTEMS_REFERENCE.md](../SYSTEMS_REFERENCE.md) — systems catalogue
