#pragma once

#include <cmath>
#include <entt.hpp>
#include <limits>
#include <string>
#include <vector>

#include "ofMain.h"
#include "ofxAssimpModelLoader.h"
#include "myNode.h"

// Uses standard openFrameworks types directly (ofSoundPlayer, ofImage, ofVideoPlayer, etc.)

// ============================================================================
// BASE COMPONENTS - Core entity data structures
// ============================================================================

namespace ecs {

extern unsigned long g_idCounter;

// ============================================================================
// Marker Components
// ============================================================================

struct app_settings {};

struct drawable {
    drawable() {}
};

struct updateable {
    updateable() {}
};

struct lightable {
    lightable(bool enabled = true) 
        : enableLighting(enabled) {}
    bool enableLighting;
};

// Scene marker component - identifies scene root entities
struct scene_component {
    scene_component() = default;
    // Marker component - scenes can still use tag_component for custom names
};

// ============================================================================
// Audio Component
// ============================================================================

struct audio_component {
    audio_component(const of::filesystem::path& path = {});
    ofSoundPlayer soundPlayer;
    of::filesystem::path audioPath;
    bool loaded = false;
};

// ============================================================================
// Camera Components
// ============================================================================

struct CameraViewPreset {
    glm::vec3 position = glm::vec3(0, 0, 300);
    glm::quat orientation = glm::quat(1, 0, 0, 0);
    glm::vec3 scale = glm::vec3(1, 1, 1);
    float nearClip = 10.0f;
    float farClip = 10000.0f;
    bool initialized = false;
    
    bool isValid() const {
        auto validVec3 = [](const glm::vec3& v) {
            return !std::isnan(v.x) && !std::isnan(v.y) && !std::isnan(v.z) &&
                   !std::isinf(v.x) && !std::isinf(v.y) && !std::isinf(v.z);
        };
        auto validQuat = [](const glm::quat& q) {
            return !std::isnan(q.x) && !std::isnan(q.y) && !std::isnan(q.z) && !std::isnan(q.w) &&
                   !std::isinf(q.x) && !std::isinf(q.y) && !std::isinf(q.z) && !std::isinf(q.w);
        };
        return validVec3(position) && validQuat(orientation) && validVec3(scale) &&
               !std::isnan(nearClip) && !std::isnan(farClip) &&
               !std::isinf(nearClip) && !std::isinf(farClip);
    }
};

struct camera_component {
    camera_component(bool active = false);
    ofCamera camera;
    bool isActive;
    bool drawFrustum;
    bool transformDirty = false;  // Set when camera transform changes
    CameraViewPreset presets[3];  // 0=2D, 1=Ortho, 2=Perspective
    
    void saveToPreset(int modeIndex);
    void loadFromPreset(int modeIndex);
    void clearDirty() { transformDirty = false; }
};

// ============================================================================
// FBO Component
// ============================================================================

struct fbo_component {
    fbo_component(std::string name = "Canvas");
    ofFbo fbo;
    ofColor clearColor;
    bool clearFrame;
    bool dirty = true;
    
    // FBO settings (editable)
    int width = 800;
    int height = 600;
    int internalFormat = GL_RGBA;  // GL_RGBA, GL_RGB, GL_RGBA8, GL_RGBA16F, GL_RGBA32F, etc.
    
    ofRectangle getViewport();
    
    void setClearColor(const ofColor& color) {
        if (clearColor != color) {
            clearColor = color;
            dirty = true;
        }
    }
    
    // Reallocate FBO with current settings
    void reallocate() {
        fbo.allocate(width, height, internalFormat);
        fbo.getTexture().setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
        dirty = true;
    }
    
    // Update settings and reallocate if changed
    void setSize(int w, int h) {
        if (width != w || height != h) {
            width = w;
            height = h;
            reallocate();
        }
    }
    
    void setInternalFormat(int format) {
        if (internalFormat != format) {
            internalFormat = format;
            reallocate();
        }
    }
};

// ============================================================================
// Fresh Component (dirty tracking)
// ============================================================================

struct fresh_component {
    bool m_bFresh;
    uint64_t m_iLastWashed;
    
    void setFresh(bool fresh);
    bool isFresh() const;
    uint64_t lastWashed() const;
};

// ============================================================================
// Filepath Component
// ============================================================================

struct filepath_component {
    filepath_component() = default;
    filepath_component(const std::string& p) : path(p) {}
    filepath_component(const of::filesystem::path& p) : path(p) {}
    
    of::filesystem::path path;
    
    std::string getFileName() const { return path.filename().stem().string(); }
    std::string getExtension() const { return path.filename().extension().string(); }
    std::string getFullName() const { return path.filename().string(); }
    std::string getString() const { return path.string(); }
    std::string getDirectory() const { return path.parent_path().string(); }
    bool exists() const { return of::filesystem::exists(path); }
    bool isEmpty() const { return path.empty(); }
};

// ============================================================================
// Image Component
// ============================================================================

struct image_component {
    image_component(const of::filesystem::path& path = {});
    ofImage image;
};

// ============================================================================
// Mesh Component
// ============================================================================

enum eMeshPrimitiveType {
    MESH_CUSTOM = 0,
    MESH_BOX,
    MESH_SPHERE,
    MESH_CONE,
    MESH_CYLINDER,
    MESH_PLANE,
    MESH_ICOSPHERE,
    MESH_ICOSAHEDRON,
    MESH_SKYBOX
};

struct mesh_component {
    ofMesh m_mesh;
    eMeshPrimitiveType primitiveType = MESH_CUSTOM;
    
    float width = 100.0f;
    float height = 100.0f;
    float depth = 100.0f;
    float radius = 50.0f;
    int resolution = 1;
    
    ofColor color = ofColor(200, 200, 200);
    bool drawWireframe = false;
    bool drawFaces = true;
    
    void rebuild();
    static ofMesh createSkyboxMesh();
};

// ============================================================================
// Model Component
// ============================================================================

struct model_component {
    model_component(std::string path = "");
    ofxAssimpModelLoader model;
    ofMaterial material;
    bool useMaterial;
};

// ============================================================================
// Node Component
// ============================================================================

struct node_component {
    node_component();
    node_component(std::string name);
    node_component(glm::vec3 position);
    myNode node;  // Extended ofNode with transform events
    unsigned long id = 0;  // Unique ID for this node (uses ecs::g_idCounter)
    bool childrenAllowed = true;   // If false, this node cannot have children (e.g. Camera, Light)

    // Reflection cache: stable-address mirror of node's transform, used by
    // ofxBapp's ComponentInspector::addReflectable (which stores raw pointers).
    // ofNode::getPosition()/getScale() return by value, so they cannot be
    // bound to a raw float* for long-term access. These fields are populated
    // from the node at construction; ofxBapp provides pullNodeTransformCaches()
    // and pushNodeTransformCaches() helpers to keep them in sync around
    // reflection read/write batches (state morph, patch cords, snapshots).
    glm::vec3 cachedPosition{0.f};
    glm::vec3 cachedScale{1.f};

    // Wrappers - delegate to myNode
    void clearDirty() { node.clearDirty(); }
    unsigned long getId() const { return id; }
    const std::string& getName() const { return node.getName(); }
    void setName(const std::string& n) { node.setName(n); }
};

// ============================================================================
// Parent/Hierarchy Component
// ============================================================================

struct parent_component {
    parent_component(entt::entity parent = entt::null);
    entt::entity parent;
    std::vector<entt::entity> children;
    
    void removeChild(entt::entity child) {
        children.erase(std::remove(children.begin(), children.end(), child), children.end());
    }
};

// Destroy handlers - called when entities with these components are destroyed
void onParentDestroy(entt::registry& registry, entt::entity entity);
void onCameraDestroy(entt::registry& registry, entt::entity entity);

// Active camera helpers - for apps that need the current camera
ofCamera* getActiveCamera(entt::registry& registry);
entt::entity getActiveCameraEntity(entt::registry& registry);
void setActiveCamera(entt::registry& registry, ofCamera* camera);

// ============================================================================
// Render Component
// ============================================================================

struct render_component {
    render_component(int d = 0, bool lighting = false, bool vis = true);
    int order;
    bool enableLighting = false;
    bool visible = true;
};

// ============================================================================
// Resource Component
// ============================================================================

enum eResourceStatus {
    ERS_OK = 1,
    ERS_MISSING,
    ERS_COUNT
};

enum eResourceType {
    ERT_IMAGE = 1,
    ERT_VIDEO,
    ERT_AUDIO,
    ERT_MODEL,
    ERT_CUBEMAP,
    ERT_UV_MAP,
    ERT_COUNT
};

struct resource_component {
    resource_component(eResourceType type);
    eResourceType m_eType;
    eResourceStatus status;
    
    eResourceStatus checkResource(entt::registry& registry, entt::entity entity);
    const eResourceType getType() const;
};

// ============================================================================
// Selectable Component
// ============================================================================

struct selectable_component {
    selectable_component(bool sel = false);
    bool selected;
};

inline void clearSelection(entt::registry& registry)
{
    for (auto [entity, sel] : registry.view<selectable_component>().each())
        sel.selected = false;
}

inline void selectEntity(entt::registry& registry, entt::entity entity)
{
    clearSelection(registry);

    if (entity == entt::null || !registry.valid(entity))
        return;

    if (!registry.all_of<selectable_component>(entity))
        registry.emplace<selectable_component>(entity, false);

    registry.get<selectable_component>(entity).selected = true;
}

inline void ensureSelectable(entt::registry& registry, entt::entity entity)
{
    if (entity == entt::null || !registry.valid(entity))
        return;
    if (!registry.all_of<selectable_component>(entity))
        registry.emplace<selectable_component>(entity, false);
}

inline void selectEntityAdd(entt::registry& registry, entt::entity entity)
{
    if (entity == entt::null || !registry.valid(entity))
        return;
    ensureSelectable(registry, entity);
    registry.get<selectable_component>(entity).selected = true;
}

inline void selectEntityToggle(entt::registry& registry, entt::entity entity)
{
    if (entity == entt::null || !registry.valid(entity))
        return;
    ensureSelectable(registry, entity);
    auto& sel = registry.get<selectable_component>(entity);
    sel.selected = !sel.selected;
}

inline void getSelectedEntities(entt::registry& registry,
                                 std::vector<entt::entity>& out)
{
    out.clear();
    for (auto [entity, sel] : registry.view<selectable_component>().each()) {
        if (sel.selected)
            out.push_back(entity);
    }
}

inline entt::entity getSelectedEntity(entt::registry& registry)
{
    for (auto [entity, sel] : registry.view<selectable_component>().each()) {
        if (sel.selected)
            return entity;
    }
    return entt::null;
}

/// Axis-aligned half-extents for mesh_component primitives (local space, centered at origin).
inline glm::vec3 meshLocalHalfExtents(const mesh_component& mesh)
{
    switch (mesh.primitiveType) {
        case MESH_SPHERE:
        case MESH_ICOSPHERE:
        case MESH_ICOSAHEDRON:
            return glm::vec3(mesh.radius);
        case MESH_CONE:
        case MESH_CYLINDER:
            return glm::vec3(mesh.radius, mesh.height * 0.5f, mesh.radius);
        case MESH_PLANE:
            return glm::vec3(mesh.width * 0.5f, 1.f, mesh.height * 0.5f);
        default:
            return glm::vec3(mesh.width, mesh.height, mesh.depth) * 0.5f;
    }
}

inline bool rayIntersectsSymmetricAABB(const glm::vec3& origin,
                                     const glm::vec3& dir,
                                     const glm::vec3& halfExtents,
                                     float& tHit)
{
    const float eps = 1e-6f;
    glm::vec3 invDir;
    for (int i = 0; i < 3; ++i) {
        if (std::fabs(dir[i]) < eps)
            invDir[i] = std::numeric_limits<float>::max();
        else
            invDir[i] = 1.f / dir[i];
    }

    const glm::vec3 t0 = (-halfExtents - origin) * invDir;
    const glm::vec3 t1 = (halfExtents - origin) * invDir;
    const glm::vec3 tmin = glm::min(t0, t1);
    const glm::vec3 tmax = glm::max(t0, t1);

    const float tNear = glm::max(glm::max(tmin.x, tmin.y), tmin.z);
    const float tFar  = glm::min(glm::min(tmax.x, tmax.y), tmax.z);
    if (tFar < 0.f || tNear > tFar)
        return false;

    tHit = tNear >= 0.f ? tNear : tFar;
    return true;
}

/// Raycast against selectable mesh entities (shared by all pick entry points).
inline entt::entity pickSelectableEntityRay(entt::registry& registry,
                                              const glm::vec3& w0,
                                              const glm::vec3& dir)
{
    if (glm::length(dir) < 1e-6f)
        return entt::null;

    entt::entity best     = entt::null;
    float        bestDist = std::numeric_limits<float>::max();

    auto view = registry.view<node_component, mesh_component, selectable_component, render_component>();
    for (auto entity : view) {
        const auto& render = view.get<render_component>(entity);
        if (!render.visible)
            continue;

        const auto& node = view.get<node_component>(entity);
        const auto& mesh = view.get<mesh_component>(entity);

        const glm::mat4 inv = glm::inverse(node.node.getGlobalTransformMatrix());
        glm::vec3       o   = glm::vec3(inv * glm::vec4(w0, 1.f));
        glm::vec3       d   = glm::vec3(inv * glm::vec4(dir, 0.f));
        const float     dLen = glm::length(d);
        if (dLen < 1e-6f)
            continue;
        d /= dLen;

        const glm::vec3 half = meshLocalHalfExtents(mesh);
        float           t    = 0.f;
        if (!rayIntersectsSymmetricAABB(o, d, half, t))
            continue;

        const glm::vec3 hitLocal  = o + d * t;
        const glm::vec3 hitWorld  = glm::vec3(node.node.getGlobalTransformMatrix() * glm::vec4(hitLocal, 1.f));
        const float     worldDist = glm::length(hitWorld - w0);
        if (worldDist < bestDist) {
            bestDist = worldDist;
            best     = entity;
        }
    }

    return best;
}

/// Pick using explicit view + projection (e.g. matrices captured during cam.begin()).
inline entt::entity pickSelectableEntity(entt::registry& registry,
                                         const glm::mat4& viewMat,
                                         const glm::mat4& projMat,
                                         glm::vec2 screenPx,
                                         const ofRectangle& viewport)
{
    if (viewport.width <= 0.f || viewport.height <= 0.f)
        return entt::null;

    const glm::mat4 mvp = projMat * viewMat;
    const float     x   = 2.f * (screenPx.x - viewport.x) / viewport.width - 1.f;
    const float     y   = 1.f - 2.f * (screenPx.y - viewport.y) / viewport.height;

    auto unproject = [&](float ndcZ) {
        const glm::vec4 h = glm::inverse(mvp) * glm::vec4(x, y, ndcZ, 1.f);
        return glm::vec3(h) / h.w;
    };

    const glm::vec3 w0  = unproject(0.f);
    const glm::vec3 w1  = unproject(1.f);
    glm::vec3       dir = w1 - w0;
    const float     dirLen = glm::length(dir);
    if (dirLen < 1e-6f)
        return entt::null;
    dir /= dirLen;

    return pickSelectableEntityRay(registry, w0, dir);
}

// ofCamera-based picking lives in ofxKit (CameraPickUtil.h) — uses getModelViewProjectionMatrix.

// ============================================================================
// Audio Settings Component
// ============================================================================

struct audio_settings_component {
    audio_settings_component() = default;
    float volume = 1.0f;
    float pan = 0.5f;
    bool isPaused = false;
};

// ============================================================================
// Sound Stream Components
// ============================================================================

struct soundStream_component {
    soundStream_component(ofSoundStreamSettings settings);
    ofSoundStream soundStream;
};

struct soundStream_settings_component {
    soundStream_settings_component();
    ofSoundStreamSettings settings;
};

// ============================================================================
// Tag Component
// ============================================================================

struct tag_component {
    tag_component(std::string t = "");
    std::string tag;
};

// ============================================================================
// Code snippet (G-code, scripts, plain text — edited in Properties)
// ============================================================================

enum class code_language {
    None,
    Gcode,
    PlainText
};

struct code_snippet_component {
    code_snippet_component(std::string t = "", code_language lang = code_language::Gcode);
    std::string text;
    code_language language = code_language::Gcode;
    bool readOnly = false;
};

// ============================================================================
// Text Component
// ============================================================================

struct text_component {
    text_component(std::string t = "");
    ofTrueTypeFont font;
    std::string text;
};

// ============================================================================
// Video Component
// ============================================================================

struct video_component {
    video_component(const of::filesystem::path& path = {});
    ofVideoPlayer videoPlayer;
};

// ============================================================================
// FBO Reference Component (Canvas Instance)
// ============================================================================

struct fbo_reference_component {
    entt::entity sourceEntity = entt::null;  // FBO entity to draw
    bool showBorder = false;                  // Debug: show instance bounds
    
    bool isValid(entt::registry& reg) const {
        return sourceEntity != entt::null && 
               reg.valid(sourceEntity) && 
               reg.any_of<fbo_component>(sourceEntity);
    }
};

// ---------------------------------------------------------------------------
// Mesh vertex picking (screen-space nearest vertex)
// ---------------------------------------------------------------------------

/// Nearest vertex of @p mesh in screen space. @p world is the entity node matrix.
/// Returns vertex index or -1; @p outDistPx is the pixel distance (always set).
inline int pickMeshVertexScreen(const ofMesh& mesh,
                                const glm::mat4& world,
                                const ofCamera& cam,
                                glm::vec2 screenPx,
                                const ofRectangle& viewport,
                                float maxPx,
                                float& outDistPx)
{
    const int n = mesh.getNumVertices();
    if (n <= 0) {
        outDistPx = maxPx + 1.f;
        return -1;
    }

    int   best     = -1;
    float bestDist = maxPx + 1.f;

    for (int i = 0; i < n; ++i) {
        const glm::vec3 local = mesh.getVertex(i);
        const glm::vec3 wpos  = glm::vec3(world * glm::vec4(local, 1.f));
        const glm::vec3 scr   = cam.worldToScreen(wpos, viewport);
        const float     d     = glm::distance(glm::vec2(scr.x, scr.y), screenPx);
        if (d < bestDist) {
            bestDist = d;
            best     = i;
        }
    }

    outDistPx = bestDist;
    return (best >= 0 && bestDist <= maxPx) ? best : -1;
}

/// Pick the nearest mesh vertex among selectable mesh entities.
inline std::pair<entt::entity, int> pickMeshVertexEntity(entt::registry& registry,
                                                         const ofCamera& cam,
                                                         glm::vec2 screenPx,
                                                         const ofRectangle& viewport,
                                                         float maxPx)
{
    entt::entity bestEnt  = entt::null;
    int          bestIdx  = -1;
    float        bestDist = maxPx + 1.f;

    auto view = registry.view<node_component, mesh_component, selectable_component, render_component>();
    for (auto entity : view) {
        const auto& render = view.get<render_component>(entity);
        if (!render.visible)
            continue;

        const auto& node = view.get<node_component>(entity);
        const auto& mesh = view.get<mesh_component>(entity);

        float dist = 0.f;
        const int idx = pickMeshVertexScreen(mesh.m_mesh,
                                             node.node.getGlobalTransformMatrix(),
                                             cam,
                                             screenPx,
                                             viewport,
                                             maxPx,
                                             dist);
        if (idx >= 0 && dist < bestDist) {
            bestDist = dist;
            bestEnt  = entity;
            bestIdx  = idx;
        }
    }

    return {bestEnt, bestIdx};
}

} // namespace ecs
