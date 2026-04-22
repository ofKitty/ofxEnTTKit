#pragma once

#include <cmath>
#include <entt.hpp>
#include <filesystem>
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
    audio_component(const std::filesystem::path& path = "");
    ofSoundPlayer soundPlayer;
    std::filesystem::path audioPath;
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
    const bool isFresh();
    const uint64_t lastWashed();
};

// ============================================================================
// Filepath Component
// ============================================================================

struct filepath_component {
    filepath_component() = default;
    filepath_component(const std::string& p) : path(p) {}
    filepath_component(const std::filesystem::path& p) : path(p) {}
    
    std::filesystem::path path;
    
    std::string getFileName() const { return path.filename().stem().string(); }
    std::string getExtension() const { return path.filename().extension().string(); }
    std::string getFullName() const { return path.filename().string(); }
    std::string getString() const { return path.string(); }
    std::string getDirectory() const { return path.parent_path().string(); }
    bool exists() const { return std::filesystem::exists(path); }
    bool isEmpty() const { return path.empty(); }
};

// ============================================================================
// Image Component
// ============================================================================

struct image_component {
    image_component(const std::filesystem::path& path = "");
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
    video_component(const std::filesystem::path& path = "");
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

} // namespace ecs
