#include "mesh_render_system.h"

namespace ecs {
using namespace ecs;

void MeshRenderSystem::draw(entt::registry& registry) {
    // Mesh rendering is handled by ofxBapp's renderEntity for proper transform handling
}

// ============================================================================
// Mesh Drawing
// ============================================================================

void MeshRenderSystem::drawMesh(const mesh_component& comp, material_component* mat) {
    ofPushStyle();
    
    if (mat) {
        mat->begin();
    } else {
        ofSetColor(comp.color);
    }
    
    if (comp.drawFaces) {
        comp.m_mesh.draw();
    }
    
    if (mat) {
        mat->end();
    }
    
    if (comp.drawWireframe) {
        ofSetColor(comp.color.r * 0.3f, comp.color.g * 0.3f, comp.color.b * 0.3f);
        comp.m_mesh.drawWireframe();
    }
    
    ofPopStyle();
}

// ============================================================================
// Model Drawing - model only has model, material, useMaterial
// ============================================================================

void MeshRenderSystem::drawModel(model_component & comp) {
    if (comp.useMaterial) {
        // Use custom material but keep model's textures and transformations
        comp.material.begin();
        for (int i = 0; i < comp.model.getMeshCount(); i++) {
            // Apply mesh's internal transformation matrix
            ofPushMatrix();
            ofMultMatrix(comp.model.getMeshHelper(i).matrix);
            
            // Bind texture if available
            ofTexture tex = comp.model.getTextureForMesh(i);
            if (tex.isAllocated()) {
                tex.bind();
            }
            
            // Draw mesh
            comp.model.getMesh(i).draw();
            
            // Unbind texture
            if (tex.isAllocated()) {
                tex.unbind();
            }
            
            ofPopMatrix();
        }
        comp.material.end();
    } else {
        // Use model's built-in materials and textures from the file
        comp.model.drawFaces();
    }
}

// ============================================================================
// Primitive Creation - Box
// Uses mesh's: width, height, depth, primitiveType
// ============================================================================

void MeshRenderSystem::createBox(mesh_component & comp, float width, float height, float depth) {
    comp.m_mesh = ofMesh::box(width, height, depth, 1, 1, 1);
    comp.m_mesh.enableNormals();
    comp.primitiveType = MESH_BOX;
    comp.width = width;
    comp.height = height;
    comp.depth = depth;
}

// ============================================================================
// Primitive Creation - Sphere
// ============================================================================

void MeshRenderSystem::createSphere(mesh_component & comp, float radius, int resolution) {
    comp.m_mesh = ofMesh::sphere(radius, resolution);
    comp.m_mesh.enableNormals();
    comp.primitiveType = MESH_SPHERE;
    comp.radius = radius;
    comp.resolution = resolution;
}

// ============================================================================
// Primitive Creation - Cylinder
// ============================================================================

void MeshRenderSystem::createCylinder(mesh_component & comp, float radius, float height, int resolution) {
    comp.m_mesh = ofMesh::cylinder(radius, height, resolution, 2, 2, true);
    comp.m_mesh.enableNormals();
    comp.primitiveType = MESH_CYLINDER;
    comp.radius = radius;
    comp.height = height;
    comp.resolution = resolution;
}

// ============================================================================
// Primitive Creation - Cone
// ============================================================================

void MeshRenderSystem::createCone(mesh_component & comp, float radius, float height, int resolution) {
    comp.m_mesh = ofMesh::cone(radius, height, resolution, 2, OF_PRIMITIVE_TRIANGLES);
    comp.m_mesh.enableNormals();
    comp.primitiveType = MESH_CONE;
    comp.radius = radius;
    comp.height = height;
    comp.resolution = resolution;
}

// ============================================================================
// Primitive Creation - Plane
// ============================================================================

void MeshRenderSystem::createPlane(mesh_component & comp, float width, float height, int columns, int rows) {
    comp.m_mesh = ofMesh::plane(width, height, columns, rows, OF_PRIMITIVE_TRIANGLES);
    comp.m_mesh.enableNormals();
    comp.primitiveType = MESH_PLANE;
    comp.width = width;
    comp.height = height;
    comp.resolution = columns;  // Use resolution for columns
}

// ============================================================================
// Primitive Creation - IcoSphere
// ============================================================================

void MeshRenderSystem::createIcoSphere(mesh_component & comp, float radius, int iterations) {
    comp.m_mesh = ofMesh::icosphere(radius, iterations);
    comp.m_mesh.enableNormals();
    comp.primitiveType = MESH_ICOSPHERE;
    comp.radius = radius;
    comp.resolution = iterations;
}

// ============================================================================
// Primitive Creation - Skybox Mesh
// ============================================================================

void MeshRenderSystem::createSkyboxMesh(mesh_component & comp) {
    // Use the static helper from mesh    comp.m_mesh = mesh::createSkyboxMesh();
    comp.primitiveType = MESH_CUSTOM;
}

// ============================================================================
// Primitive Drawing
// ============================================================================

void MeshRenderSystem::drawPrimitive(primitive_component & comp) {
    if (comp.needsRebuild) comp.rebuild();
    comp.primitive.draw();
}

// ============================================================================
// Billboard Drawing
// ============================================================================

void MeshRenderSystem::drawBillboard(const billboard_component & comp) {
    ofPushStyle();
    ofSetColor(comp.tint, comp.tint.a * comp.alpha);
    
    if (comp.texture.isAllocated()) {
        comp.texture.draw(-comp.width/2, -comp.height/2, comp.width, comp.height);
    } else {
        ofDrawRectangle(-comp.width/2, -comp.height/2, comp.width, comp.height);
    }
    
    ofPopStyle();
}

// ============================================================================
// Trail Drawing
// ============================================================================

void MeshRenderSystem::drawTrail(const trail_component & comp) {
    if (comp.points.size() < 2) return;
    
    ofPushStyle();
    
    for (size_t i = 1; i < comp.points.size(); i++) {
        const auto& p0 = comp.points[i-1];
        const auto& p1 = comp.points[i];
        
        ofSetColor(p0.color.getLerped(p1.color, 0.5f));
        ofSetLineWidth((p0.width + p1.width) * 0.5f);
        
        ofDrawLine(p0.position, p1.position);
    }
    
    ofPopStyle();
}

// ============================================================================
// Instanced Mesh Drawing
// ============================================================================
// TODO: CHECK
void MeshRenderSystem::drawInstancedMesh(const instanced_mesh_component & comp) {
    ofPushStyle();
    
    for (size_t i = 0; i < comp.transforms.size(); i++) {
        ofPushMatrix();
        ofMultMatrix(comp.transforms[i]);
        
        if (comp.useColors && i < comp.colors.size()) {
            ofSetColor(comp.colors[i]);
        }
        
        comp.vboMesh.draw();
        
        ofPopMatrix();
    }
    
    ofPopStyle();
}

// ============================================================================
// Tube Drawing
// ============================================================================

void MeshRenderSystem::drawTube(tube_component & comp) {
    if (comp.needsRebuild) comp.rebuild();
    
    ofPushStyle();
    ofSetColor(comp.color);
    comp.mesh.draw();
    ofPopStyle();
}

// ============================================================================
// Surface Drawing
// ============================================================================

void MeshRenderSystem::drawSurface(surface_component & comp) {
    if (comp.needsRebuild) comp.rebuild();
    
    ofPushStyle();
    ofSetColor(comp.color);
    
    if (comp.wireframe) {
        comp.mesh.drawWireframe();
    } else {
        comp.mesh.draw();
    }
    
    ofPopStyle();
}

} // namespace ecs
