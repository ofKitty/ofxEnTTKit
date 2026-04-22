#include "base_components.h"

namespace ecs {

unsigned long g_idCounter = 0;

// ============================================================================
// audio_component
// ============================================================================

audio_component::audio_component(const std::filesystem::path& path) : audioPath(path) {
    if (!path.empty()) {
        loaded = soundPlayer.load(path.string());
        if (!loaded) {
            ofLogError("audio_component") << "Failed to load audio: " << path.string();
        }
    }
}

// ============================================================================
// camera_component
// ============================================================================

camera_component::camera_component(bool active) {
    camera.clearParent();
    camera.resetTransform();
    camera.setFov(60);
    camera.setNearClip(0.1f);
    camera.setFarClip(2000);
    camera.setForceAspectRatio(true);
    camera.setAspectRatio(16.0f / 9.0f);
    camera.setPosition(0, 0, 250);
    isActive = active;
    drawFrustum = false;
}

void camera_component::saveToPreset(int modeIndex) {
    if (modeIndex < 0 || modeIndex > 2) return;
    CameraViewPreset temp;
    temp.position = camera.getPosition();
    temp.orientation = camera.getOrientationQuat();
    temp.scale = camera.getScale();
    temp.nearClip = camera.getNearClip();
    temp.farClip = camera.getFarClip();
    temp.initialized = true;
    
    if (temp.isValid()) {
        presets[modeIndex] = temp;
    }
}

void camera_component::loadFromPreset(int modeIndex) {
    if (modeIndex < 0 || modeIndex > 2) return;
    auto& p = presets[modeIndex];
    if (!p.initialized || !p.isValid()) return;
    camera.setPosition(p.position);
    camera.setOrientation(p.orientation);
    camera.setScale(p.scale);
    camera.setNearClip(p.nearClip);
    camera.setFarClip(p.farClip);
}

// ============================================================================
// fbo_component
// ============================================================================

fbo_component::fbo_component(std::string name) {
    clearColor = ofColor(255, 255, 255, 255);
    clearFrame = true;
    width = 800;
    height = 600;
    internalFormat = GL_RGBA;
    fbo.allocate(width, height, internalFormat);
    fbo.getTexture().setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
}

ofRectangle fbo_component::getViewport() {
    return ofRectangle(0, 0, fbo.getWidth(), fbo.getHeight());
}

// ============================================================================
// fresh_component
// ============================================================================

void fresh_component::setFresh(bool fresh) {
    m_bFresh = fresh;
    if (fresh) m_iLastWashed = ofGetFrameNum();
}

const bool fresh_component::isFresh() {
    return m_bFresh;
}

const uint64_t fresh_component::lastWashed() {
    return m_iLastWashed;
}

// ============================================================================
// image_component
// ============================================================================

image_component::image_component(const std::filesystem::path& path) {
    if (!path.empty()) {
        if (image.load(path)) {
            image.getTexture().setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
            ofLogNotice("image_component") << "Loaded image: " << path.string();
        } else {
            ofLogError("image_component") << "Failed to load image: " << path.string();
        }
    }
}

// ============================================================================
// node_component
// ============================================================================

node_component::node_component() : node(glm::vec3(0), "Node"), id(++g_idCounter) {
    cachedPosition = node.getPosition();
    cachedScale    = node.getScale();
}

node_component::node_component(std::string n) : node(glm::vec3(0), n), id(++g_idCounter) {
    cachedPosition = node.getPosition();
    cachedScale    = node.getScale();
}

node_component::node_component(glm::vec3 position) : node(position, "Node"), id(++g_idCounter) {
    cachedPosition = node.getPosition();
    cachedScale    = node.getScale();
}

// ============================================================================
// parent_component
// ============================================================================

parent_component::parent_component(entt::entity p)
    : parent(p) {}

// ============================================================================
// Destroy handlers
// ============================================================================

void onParentDestroy(entt::registry& registry, entt::entity entity) {
    if (registry.any_of<parent_component>(entity)) {
        auto& parentComp = registry.get<parent_component>(entity);
        std::vector<entt::entity> childrenCopy = parentComp.children;
        for (auto child : childrenCopy) {
            if (registry.valid(child)) {
                registry.destroy(child);
            }
        }
    }

    if (registry.any_of<parent_component>(entity)) {
        auto& parentComp = registry.get<parent_component>(entity);
        if (parentComp.parent != entt::null && registry.valid(parentComp.parent)) {
            auto& parentParentComp = registry.get<parent_component>(parentComp.parent);
            parentParentComp.removeChild(entity);
        }
    }
}

void onCameraDestroy(entt::registry& registry, entt::entity entity) {
    if (registry.any_of<camera_component>(entity)) {
        auto& cam = registry.get<camera_component>(entity);
        if (cam.isActive) {
            auto cameras = registry.view<camera_component>();
            for (auto camEntity : cameras) {
                if (camEntity != entity) {
                    auto& replacement = registry.get<camera_component>(camEntity);
                    replacement.isActive = true;
                    break;
                }
            }
        }
    }
}

// ============================================================================
// Active camera helpers
// ============================================================================

ofCamera* getActiveCamera(entt::registry& registry) {
    auto entity = getActiveCameraEntity(registry);
    if (entity == entt::null) return nullptr;
    return &registry.get<camera_component>(entity).camera;
}

entt::entity getActiveCameraEntity(entt::registry& registry) {
    auto cameras = registry.view<camera_component>();
    for (auto entity : cameras) {
        if (registry.get<camera_component>(entity).isActive) {
            return entity;
        }
    }
    return entt::null;
}

void setActiveCamera(entt::registry& registry, ofCamera* camera) {
    auto cameras = registry.view<camera_component>();
    for (auto entity : cameras) {
        auto& comp = registry.get<camera_component>(entity);
        comp.isActive = (camera != nullptr && &comp.camera == camera);
    }
}

// ============================================================================
// render_component
// ============================================================================

render_component::render_component(int d, bool lighting, bool vis)
    : order(d), enableLighting(lighting), visible(vis) {}

// ============================================================================
// resource_component
// ============================================================================

resource_component::resource_component(eResourceType type)
    : m_eType(type), status(ERS_OK) {}

eResourceStatus resource_component::checkResource(entt::registry& registry, entt::entity entity) {
    if (auto* pathComp = registry.try_get<filepath_component>(entity)) {
        status = pathComp->exists() ? ERS_OK : ERS_MISSING;
    } else {
        status = ERS_MISSING;
    }
    return status;
}

const eResourceType resource_component::getType() const {
    return m_eType;
}

// ============================================================================
// selectable_component
// ============================================================================

selectable_component::selectable_component(bool sel)
    : selected(sel) {}

// ============================================================================
// soundStream_component
// ============================================================================

soundStream_component::soundStream_component(ofSoundStreamSettings settings) {
    soundStream.printDeviceList();
    soundStream.setup(settings);
}

soundStream_settings_component::soundStream_settings_component() {
    settings.bufferSize = 512;
    settings.sampleRate = 44100;
    settings.numOutputChannels = 2;
    settings.numInputChannels = 0;
}

// ============================================================================
// tag_component
// ============================================================================

tag_component::tag_component(std::string t)
    : tag(t) {}

// ============================================================================
// text_component
// ============================================================================

text_component::text_component(std::string text)
    : text(text) {
    font.load("verdana.ttf", 12);
}

// ============================================================================
// video_component
// ============================================================================

video_component::video_component(const std::filesystem::path& path) {
    if (!path.empty()) {
        videoPlayer.load(path.string());
        videoPlayer.setUseTexture(true);
        videoPlayer.setLoopState(OF_LOOP_NORMAL);
    }
}

// ============================================================================
// model_component
// ============================================================================

model_component::model_component(std::string path)
    : useMaterial(false) {
    material.setDiffuseColor(ofColor(255, 255, 255));
    material.setAmbientColor(ofColor(50, 50, 50));
    material.setSpecularColor(ofColor(255, 255, 255));
    material.setShininess(64.0f);
    
    if (!path.empty()) {
        if (model.load(path)) {
            for (unsigned int meshNum = 0; meshNum < model.getMeshCount(); meshNum++) {
                model.getTextureForMesh(meshNum).setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
            }
            model.update();
        }
    }
}

// ============================================================================
// mesh_component
// ============================================================================

void mesh_component::rebuild() {
    switch (primitiveType) {
        case MESH_BOX:
            m_mesh = ofMesh::box(width, height, depth, resolution, resolution, resolution);
            break;
        case MESH_SPHERE:
            m_mesh = ofMesh::sphere(radius, resolution > 0 ? resolution * 12 : 12);
            break;
        case MESH_CONE:
            m_mesh = ofMesh::cone(radius, height, resolution > 0 ? resolution * 12 : 12, 2);
            break;
        case MESH_CYLINDER:
            m_mesh = ofMesh::cylinder(radius, height, resolution > 0 ? resolution * 12 : 12, 2);
            break;
        case MESH_PLANE:
            m_mesh = ofMesh::plane(width, height, resolution > 0 ? resolution * 10 : 10, resolution > 0 ? resolution * 10 : 10);
            break;
        case MESH_ICOSPHERE:
            m_mesh = ofMesh::icosphere(radius, resolution > 0 ? resolution : 2);
            break;
        case MESH_ICOSAHEDRON:
            m_mesh = ofMesh::icosahedron(radius);
            break;
        case MESH_SKYBOX:
            m_mesh = createSkyboxMesh();
            break;
        case MESH_CUSTOM:
        default:
            break;
    }
}

ofMesh mesh_component::createSkyboxMesh() {
    ofMesh mesh;
    mesh.setMode(OF_PRIMITIVE_TRIANGLES);
    
    float v[] = {
        // Back face (z = -1)
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        // Left face (x = -1)
        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,
        // Right face (x = 1)
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        // Front face (z = 1)
        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,
        // Top face (y = 1)
        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,
        // Bottom face (y = -1)
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };
    
    for (int i = 0; i < 36; i++) {
        mesh.addVertex(glm::vec3(v[i*3], v[i*3+1], v[i*3+2]));
    }
    
    return mesh;
}

} // namespace ecs
