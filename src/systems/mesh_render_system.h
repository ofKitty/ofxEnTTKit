#pragma once

#include "base_system.h"
#include "ofMain.h"
#include "../components/base_components.h"
#include "../components/rendering_components.h"

namespace ecs {

// ============================================================================
// Mesh Render System
// ============================================================================
// Handles rendering of 3D meshes and models.
// Components are pure data - this system contains the rendering logic.

class MeshRenderSystem : public ISystem {
public:
    const char* getName() const override { return "MeshRenderSystem"; }
    
    void draw(entt::registry& registry) override;
    
    // Individual draw functions
    static void drawMesh(const mesh_component& comp, material_component* mat = nullptr);
    static void drawModel(model_component& comp);
    static void drawPrimitive(primitive_component& comp);
    static void drawBillboard(const billboard_component& comp);
    static void drawTrail(const trail_component& comp);
    static void drawInstancedMesh(const instanced_mesh_component& comp);
    static void drawTube(tube_component& comp);
    static void drawSurface(surface_component& comp);
    
    // Mesh primitive creation helpers
    // Uses mesh's existing members: width, height, depth, radius, resolution, primitiveType
    static void createBox(mesh_component& comp, float width, float height, float depth);
    static void createSphere(mesh_component& comp, float radius, int resolution);
    static void createCylinder(mesh_component& comp, float radius, float height, int resolution);
    static void createCone(mesh_component& comp, float radius, float height, int resolution);
    static void createPlane(mesh_component& comp, float width, float height, int columns, int rows);
    static void createIcoSphere(mesh_component& comp, float radius, int iterations);
    static void createSkyboxMesh(mesh_component& comp);
};

} // namespace ecs
