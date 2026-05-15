# ofxEnTTKit — Systems Reference

All systems live in `src/systems/`.  Include `systems/systems.h` to get the full set plus `ecs::ISystem` and `ecs::SystemManager`.  `TransformSystem` is included separately by the umbrella header `ofxEnTTKit.h` because it must be called **manually** outside the `SystemManager` frame loop.

---

## SystemManager / ISystem

**File:** `base_system.h`

The orchestration layer.  Every system inherits `ecs::ISystem` which declares:

| Virtual method | Called by |
|---|---|
| `setup(registry)` | `SystemManager::setup()` — once |
| `update(registry, deltaTime)` | `SystemManager::update()` — each frame |
| `draw(registry)` | `SystemManager::draw()` — each frame |
| `cleanup(registry)` | `SystemManager::cleanup()` — on teardown |
| `getName() const` | debugging / lookup |

`SystemManager::addSystem<T>(args...)` constructs T and returns a raw pointer for later access via `getSystem<T>()`.  Systems are dispatched in registration order — register `LayerSystem` first so ordering data is ready when render systems run.

---

## TransformSystem  ⚠ NOT in SystemManager

**File:** `systems/TransformSystem.h` / `.cpp`  
**Must be called manually** — it is deliberately excluded from `systems.h` because transform propagation must happen at a specific point in the frame (before anything reads `GlobalTransform`).  `ofxEnTTKit.h` includes it separately.

**Reads:** `ecs::Relationship`, `ecs::LocalTransform`  
**Writes:** `ecs::GlobalTransform::matrix`

```cpp
// Call once per frame before any system reads GlobalTransform:
TransformSystem::update(registry);
```

Walks the `Relationship` tree roots-first.  Root entities (no parent) copy their `LocalTransform` directly to `GlobalTransform`.  Child entities multiply `parent_global * local`.

---

## LayerSystem

**File:** `systems/layer_system.h` / `.cpp`  
**Participates in SystemManager:** yes — register **first** so render systems can consume sorted lists.

**Reads:** `ecs::layer_component` (index, visible)  
**Writes:** internal sorted caches only

```cpp
auto* layers = systems.addSystem<ecs::LayerSystem>();
// After systems.update():
for (entt::entity e : layers->visibleLayers()) { /* render */ }
```

Rebuilds two sorted vectors each frame:
- `visibleLayers()` — entities with `visible == true`, sorted by `index` ascending
- `allLayers()` — all layer entities sorted by `index` ascending

**No rendering.** Consumers (render systems, domain engines) call these lists rather than re-querying and re-sorting independently.

---

## DestroySystem

**File:** `systems/destroy_system.h` / `.cpp`  
**Participates in SystemManager:** yes (`setup` only — no per-frame work)

**Reads/Writes:** `parent_component`, `camera_component`

Registers `on_destroy` callbacks in `setup()`:
- Cascade-destroys children when a parent entity is destroyed
- Detaches child from its parent sibling list on destroy
- Reassigns the active camera when the active camera entity is destroyed

No `update` or `draw` logic.

---

## Graphics2DRender

**File:** `systems/graphics2d_system.h` / `.cpp`  
**Participates in SystemManager:** yes (framework registration only; `draw(registry)` is intentionally **empty**)

**Integration note:** Rendering is driven by **ofxBapp** / app code which handles transform application per entity.  The system exposes a large collection of **static** draw helpers and an `IShapeRenderer` / `BatchRenderer` pipeline.

Key static helpers: `drawPath`, `drawPolyline`, `drawShape2D`, `drawSVGShape`, `beginBatch` / `endBatch` (via `BatchRenderer`).  Optional Cairo/PDF vector export path.

---

## MeshRenderSystem

**File:** `systems/mesh_render_system.h` / `.cpp`  
**Participates in SystemManager:** yes (`draw(registry)` **empty** — rendering delegated to ofxBapp `renderEntity`)

**Components consumed (via static helpers):** `mesh_component`, `material_component`, `model_component`, `primitive_component`, `billboard_component`, `trail_component`, `instanced_mesh_component`, `tube_component`, `surface_component`

Static primitive builders: `createBox`, `createSphere`, `createCylinder`, `createPlane`, `createCone`.

---

## MediaRenderSystem

**File:** `systems/media_render_system.h` / `.cpp`  
**Participates in SystemManager:** yes

**Reads/Writes:** `video_component` (update: `updateVideo`); `image_component`, `fbo_component` (draw statics)

`update` calls `updateVideo` on all `video_component` entities each frame.  `draw(registry)` has a TODO stub.  Static helpers: `drawImage`, `drawVideo`, `drawFbo`.

---

## AnimationSystem

**File:** `systems/animation_system.h` / `.cpp`  
**Participates in SystemManager:** yes

**Reads/Writes:** `tween_component`, `particle_emitter_component`, `postfx_component`

- `update`: advances all tweens (`updateTween`); updates particle emitters
- `draw`: draws particle emitters

Static helpers: `ease(type, t)`, `emitParticles`, `drawParticles`, `applyPostFX`.

---

## ModulatorSystem

**File:** `systems/modulator_system.h` / `.cpp`  
**Participates in SystemManager:** yes

**Reads/Writes:** `modulator_component` (and whatever properties the modulator's bindings target)

`update` iterates all `modulator_component` entities, advances their internal time/value, then calls `applyBindings()` to write the modulated value to bound component properties.

> **Note:** The `update` override in the header omits `override` — this is a minor inconsistency but does not affect runtime behaviour.

---

## PhysicsSystem

**File:** `systems/physics_system.h` / `.cpp`  
**Participates in SystemManager:** yes

**Reads/Writes:** `rigidbody_component`, `node_component`

Simple kinematic integration: gravity, drag, velocity → position.  No collision detection.

API: `setGravity(vec3)` / `getGravity()`, static `applyForce`, `applyImpulse`, `applyTorque`.

---

## UtilityRenderSystem

**File:** `systems/utility_render_system.h` / `.cpp`  
**Participates in SystemManager:** yes

**Reads:** `grid_helper_component` + `node_component`, `bounding_box_component` + `node_component`

`draw` renders grid helpers and bounding box wireframes for any entity that carries both a spatial component and the corresponding visual helper component.

Static helpers: `drawGridHelper`, `drawBoundingBox`.

---

## GizmoSystem

**File:** `systems/gizmo_system.h` / `.cpp`  
**Participates in SystemManager:** yes

**Reads:** `gizmo_component` + `node_component`

`draw` renders transform gizmos (translate / rotate / scale handles) using `node_component` position for placement.  Static: `drawGizmo(position)`.

---

## HardwareIOSystem

**File:** `systems/hardware_io_system.h` / `.cpp`  
**Participates in SystemManager:** yes

**Reads/Writes:** `serial_component`, `osc_component`

`update` polls all serial and OSC component entities each frame.

Static helpers: serial connect / disconnect / send / receive; OSC sender/receiver setup, send/receive utilities.

---

## TrailSystem

**File:** `systems/trail_system.h` / `.cpp`  
**Participates in SystemManager:** yes

**Reads/Writes:** `trail_component` + `node_component`

- `update`: appends the current `node_component` position to the trail point buffer; manages max-length trimming
- `draw`: renders ribbon geometry; uses the camera set via `setCamera(entity)`

Static helpers: `addTrailPoint`, `updateTrail`, `drawTrailRibbon`, `clearTrail`.

---

## EffectsRenderSystem

**File:** `systems/effects_render_system.h` / `.cpp`  
**Participates in SystemManager:** yes (base `ISystem` overrides — primarily a static helper façade)

No meaningful `update` or `draw` implementation — the system exposes **static helper methods** for post-process effects:

- Shadow helpers: `beginShadow` / `endShadow`, `drawRectShadow`, `drawCircleShadow`
- Glow: `drawGlowRect`, `drawGlowCircle`
- Outline: `drawMeshOutline`

Call these statics directly from app / render code; no registry interaction required.

---

## LEDSystem

**File:** `systems/led_system.h` / `.cpp`  
**Participates in SystemManager:** yes

**Reads/Writes:** `uv_component`, `fbo_component`, `uv_sample_component`

`update`:
1. Entities with `uv_component`: samples colours from `sourceEntity`'s `fbo_component` into `uv.sampledColors`
2. Entities with `fbo_component + uv_sample_component` where `autoSample == true`: fills `sampleComp.colors`

`draw`: empty (optional app-driven preview).

Static helpers: `sampleFromTexture`, `sampleFromFbo`, `sampleFromPixels`, `drawUVPreview`, `prepareOutput` (RGB byte buffer for LED fixture output).

---

## UVRenderSystem

**File:** `systems/uv_render_system.h` / `.cpp`  
**Participates in SystemManager:** yes

**Reads:** `uv_component` + `node_component`

`draw` renders each LED fixture using the colours sampled into `uv_component.sampledColors`.  Supports a `drawableEntity` for per-LED shape override (`mesh_component` or `image_component`).

Static helpers: `drawEntity`, `drawDrawableAt`.

---

## BatchRenderer

**File:** `systems/batch_renderer.h` / `.cpp`  
**Not an ISystem** — utility class used by `Graphics2DRender`.

`begin` / `add*` / `end` / `clear` accumulate triangles and stroke geometry into a single draw call.  Used via `Graphics2DRender::beginBatch` / `endBatch`.

---

## IShapeRenderer

**File:** `systems/IShapeRenderer.h`  
**Not an ISystem** — abstract interface consumed by `Graphics2DRender`.

```cpp
virtual void setup();
virtual void drawStroke(...);
virtual void drawFill(...);
```

Implement and pass to `Graphics2DRender` to customise how 2D shapes are rendered.

---

## Frame loop ordering recommendation

```cpp
// Manual — must precede anything that reads GlobalTransform:
TransformSystem::update(registry);

// SystemManager-managed (register in this order):
systems.addSystem<ecs::LayerSystem>();        // 1. layer ordering
systems.addSystem<ecs::DestroySystem>();      // 2. entity lifecycle
systems.addSystem<ecs::ModulatorSystem>();    // 3. driven values
systems.addSystem<ecs::PhysicsSystem>();      // 4. simulation
systems.addSystem<ecs::AnimationSystem>();    // 5. tweens / particles
systems.addSystem<ecs::HardwareIOSystem>();   // 6. serial / OSC I/O
systems.addSystem<ecs::TrailSystem>();        // 7. trail update
// Render systems last:
systems.addSystem<ecs::Graphics2DRender>();
systems.addSystem<ecs::MeshRenderSystem>();
systems.addSystem<ecs::MediaRenderSystem>();
systems.addSystem<ecs::UtilityRenderSystem>();
systems.addSystem<ecs::GizmoSystem>();
systems.addSystem<ecs::LEDSystem>();
systems.addSystem<ecs::UVRenderSystem>();
```
