#pragma once
#include "ofMain.h"
#include <entt.hpp>
#include <map>
#include <memory>

// Forward declaration so header does not require ofLight.h (avoids C2079 when
// this header is compiled from projects where openFrameworks core path is not
// in the same include order, e.g. example_ESP32).
class ofLight;

// ============================================================================
// RENDERING COMPONENTS (3D Graphics)
// ============================================================================

namespace ecs {

// Forward declarations (needed for pointers in material)
struct shader_component;
struct cubemap_component;

// ============================================================================
// Light Component
// ============================================================================

struct light_component {
    enum LightType {
        POINT,
        DIRECTIONAL,
        SPOT,
        AREA
    };

    std::unique_ptr<ofLight> light;
    LightType type;

    ofColor ambientColor;
    ofColor diffuseColor;
    ofColor specularColor;

    float attenuation_constant;
    float attenuation_linear;
    float attenuation_quadratic;

    float spotCutoff;
    float spotConcentration;

    bool enabled;
    bool castShadows;
    bool drawDebug;

    light_component();
    ~light_component();
    light_component(light_component&&) noexcept = default;
    light_component& operator=(light_component&&) noexcept = default;
    light_component(const light_component&);
    light_component& operator=(const light_component&);

    void apply();
};

// ============================================================================
// Material Component
// ============================================================================

struct material_component {
    enum MaterialType {
        MATERIAL_STANDARD,
        MATERIAL_UNLIT,
        MATERIAL_ENVIRONMENT,
        MATERIAL_PBR,
        MATERIAL_CUSTOM
    };
    
    MaterialType type = MATERIAL_STANDARD;
    ofMaterial material;
    
    ofFloatColor ambient;
    ofFloatColor diffuse;
    ofFloatColor specular;
    ofFloatColor emissive;
    
    float shininess;
    float metallic;
    float roughness;
    float ao;
    
    cubemap_component* environmentMap = nullptr;
    float reflectivity = 1.0f;
    float refractiveIndex = 1.52f;
    bool useRefraction = false;
    float fresnelPower = 3.0f;
    
    shader_component* customShader = nullptr;
    
    bool useTexture;
    bool receiveShadows;
    bool castShadows;
    
    material_component()
        : type(MATERIAL_STANDARD)
        , ambient(0.2f, 0.2f, 0.2f, 1.0f)
        , diffuse(0.8f, 0.8f, 0.8f, 1.0f)
        , specular(1.0f, 1.0f, 1.0f, 1.0f)
        , emissive(0.0f, 0.0f, 0.0f, 1.0f)
        , shininess(64.0f)
        , metallic(0.0f)
        , roughness(0.5f)
        , ao(1.0f)
        , useTexture(false)
        , receiveShadows(true)
        , castShadows(true)
    {}
    
    void apply();
    void begin();
    void end();
    void applyEnvironmentUniforms(ofShader& shader, const glm::vec3& cameraPos);
    
    bool needsCustomRendering() const {
        return type == MATERIAL_ENVIRONMENT || type == MATERIAL_CUSTOM;
    }
};

// ============================================================================
// Shader Component
// ============================================================================

struct shader_component {
    ofShader shader;
    
    std::filesystem::path vertPath;
    std::filesystem::path fragPath;
    std::filesystem::path geomPath;
    
    std::map<std::string, float> floatUniforms;
    std::map<std::string, int> intUniforms;
    std::map<std::string, glm::vec2> vec2Uniforms;
    std::map<std::string, glm::vec3> vec3Uniforms;
    std::map<std::string, glm::vec4> vec4Uniforms;
    std::map<std::string, ofFloatColor> colorUniforms;
    
    bool isLoaded;
    bool autoReload;
    uint64_t lastModifiedTime;
    
    shader_component()
        : isLoaded(false)
        , autoReload(false)
        , lastModifiedTime(0)
    {}
    
    bool load(const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath = "");
    void reload();
    void checkForReload();
    void begin();
    void end();
    
    void setUniform(const std::string& name, float value) { floatUniforms[name] = value; }
    void setUniform(const std::string& name, int value) { intUniforms[name] = value; }
    void setUniform(const std::string& name, const glm::vec2& value) { vec2Uniforms[name] = value; }
    void setUniform(const std::string& name, const glm::vec3& value) { vec3Uniforms[name] = value; }
    void setUniform(const std::string& name, const glm::vec4& value) { vec4Uniforms[name] = value; }
    void setUniform(const std::string& name, const ofFloatColor& value) { colorUniforms[name] = value; }
};

// ============================================================================
// Primitive Component
// ============================================================================

struct primitive_component {
    enum PrimitiveType {
        BOX,
        SPHERE,
        CYLINDER,
        CONE,
        PLANE,
        ICOSPHERE
    };
    
    PrimitiveType type;
    float width, height, depth;
    int resolution;
    float radius;
    float radiusTop, radiusBottom;
    float cylinderHeight;
    of3dPrimitive primitive;
    bool needsRebuild;
    
    primitive_component()
        : type(BOX)
        , width(100), height(100), depth(100)
        , resolution(20)
        , radius(50)
        , radiusTop(50), radiusBottom(50)
        , cylinderHeight(100)
        , needsRebuild(true)
    {}
    
    void rebuild();
    void drawWireframe();
};

// ============================================================================
// Texture Component
// ============================================================================

struct texture_component {
    enum TextureType { DIFFUSE, NORMAL, SPECULAR, ROUGHNESS, METALLIC, AO, EMISSIVE, HEIGHT, CUSTOM };
    enum WrapMode { REPEAT, MIRRORED_REPEAT, CLAMP_TO_EDGE, CLAMP_TO_BORDER };
    enum FilterMode { NEAREST, LINEAR, LINEAR_MIPMAP_LINEAR };
    
    ofTexture texture;
    std::filesystem::path texturePath;
    TextureType type;
    
    glm::vec2 offset;
    glm::vec2 tiling;
    float rotation;
    
    WrapMode wrapModeS;
    WrapMode wrapModeT;
    FilterMode filterMode;
    float normalStrength;
    bool loaded;
    int textureUnit;
    
    texture_component()
        : type(DIFFUSE)
        , offset(0, 0), tiling(1, 1), rotation(0)
        , wrapModeS(REPEAT), wrapModeT(REPEAT)
        , filterMode(LINEAR)
        , normalStrength(1.0f)
        , loaded(false), textureUnit(0)
    {}
    
    bool load(const std::string& path);
    void setTexture(const ofTexture& tex);
    void applySettings();
    glm::mat3 getUVMatrix() const;
    void bind();
    void unbind();
    
    float getWidth() const { return texture.isAllocated() ? texture.getWidth() : 0; }
    float getHeight() const { return texture.isAllocated() ? texture.getHeight() : 0; }
    glm::vec2 getSize() const { return glm::vec2(getWidth(), getHeight()); }
};

// ============================================================================
// Cubemap Component
// ============================================================================

struct cubemap_component {
    GLuint textureID = 0;
    std::filesystem::path facePaths[6];
    std::filesystem::path equirectangularPath;
    ofTexture equirectangularTexture;
    bool useEquirectangular = false;
    float intensity = 1.0f;
    float rotation = 0.0f;
    bool loaded = false;
    int faceSize = 0;
    
    cubemap_component() = default;
    ~cubemap_component();
    
    bool loadFromFaces(const std::string& posX, const std::string& negX,
                       const std::string& posY, const std::string& negY,
                       const std::string& posZ, const std::string& negZ);
    bool loadFromFaces(const std::vector<std::string>& paths);
    bool loadFromDirectory(const std::string& directory, const std::string& extension = ".jpg");
    bool loadEquirectangular(const std::string& path);
    void bind(int textureUnit = 0);
    void unbind(int textureUnit = 0);
    
    GLuint getTextureID() const { return textureID; }
    bool isLoaded() const { return loaded; }
    int getFaceSize() const { return faceSize; }
    float getWidth() const { return useEquirectangular && equirectangularTexture.isAllocated() ? equirectangularTexture.getWidth() : (float)faceSize; }
    float getHeight() const { return useEquirectangular && equirectangularTexture.isAllocated() ? equirectangularTexture.getHeight() : (float)faceSize; }
};

// ============================================================================
// Skybox Component
// ============================================================================

struct skybox_component {
    cubemap_component* cubemap = nullptr;
    ofMesh skyboxMesh;
    bool visible = true;
    
    skybox_component();
    void setCubemap(cubemap_component* cm) { cubemap = cm; }
    bool isReady() const { return cubemap && cubemap->isLoaded() && visible; }
    static ofMesh createSkyboxMesh();
};

// ============================================================================
// Billboard Component
// ============================================================================

struct billboard_component {
    float width, height;
    ofTexture texture;
    ofColor tint;
    float alpha;
    bool spherical;
    glm::vec3 upVector;
    
    billboard_component()
        : width(100), height(100)
        , tint(255, 255, 255, 255)
        , alpha(1.0f)
        , spherical(true)
        , upVector(0, 1, 0)
    {}
    
    bool loadTexture(const std::string& path);
    void setTexture(const ofTexture& tex) {
        texture = tex;
        if (width <= 0) width = texture.getWidth();
        if (height <= 0) height = texture.getHeight();
    }
};

// ============================================================================
// Trail Component
// ============================================================================

struct trail_component {
    struct TrailPoint {
        glm::vec3 position;
        float age;
        ofColor color;
        float width;
    };
    
    std::vector<TrailPoint> points;
    int maxPoints;
    float lifetime;
    ofColor colorStart;
    ofColor colorEnd;
    float widthStart;
    float widthEnd;
    bool smooth;
    int smoothAmount;
    bool autoAddPoints;
    
    trail_component()
        : maxPoints(100)
        , lifetime(2.0f)
        , colorStart(255, 255, 255, 255)
        , colorEnd(255, 255, 255, 0)
        , widthStart(5.0f)
        , widthEnd(1.0f)
        , smooth(true)
        , smoothAmount(10)
        , autoAddPoints(true)
    {}
    
    void clear() { points.clear(); }
    size_t size() const { return points.size(); }
};

// ============================================================================
// Instanced Mesh Component
// ============================================================================

struct instanced_mesh_component {
    ofMesh mesh;
    ofVboMesh vboMesh;
    std::vector<glm::mat4> transforms;
    std::vector<ofFloatColor> colors;
    int maxInstances;
    bool useColors;
    bool needsUpdate;
    
    instanced_mesh_component()
        : maxInstances(1000)
        , useColors(false)
        , needsUpdate(true)
    {}
    
    void setMesh(const ofMesh& _mesh) { mesh = _mesh; vboMesh = _mesh; needsUpdate = true; }
    void addInstance(const glm::mat4& transform);
    void addInstance(const glm::vec3& position, const glm::quat& rotation = glm::quat(), const glm::vec3& scale = glm::vec3(1));
    void addInstance(const glm::vec3& position, const ofFloatColor& color);
    void setInstanceTransform(size_t index, const glm::mat4& transform);
    void setInstanceColor(size_t index, const ofFloatColor& color);
    void clear() { transforms.clear(); colors.clear(); needsUpdate = true; }
    size_t size() const { return transforms.size(); }
};

// ============================================================================
// Tube Component
// ============================================================================

struct tube_component {
    std::vector<glm::vec3> path;
    float radius;
    int radialResolution;
    int pathResolution;
    ofColor color;
    bool closed;
    bool caps;
    bool smooth;
    ofMesh mesh;
    bool needsRebuild;
    
    tube_component()
        : radius(10)
        , radialResolution(16)
        , pathResolution(1)
        , color(255, 255, 255, 255)
        , closed(false)
        , caps(true)
        , smooth(true)
        , needsRebuild(true)
    {}
    
    void addPoint(const glm::vec3& point) { path.push_back(point); needsRebuild = true; }
    void addPoint(float x, float y, float z) { path.push_back(glm::vec3(x, y, z)); needsRebuild = true; }
    void setPath(const std::vector<glm::vec3>& _path) { path = _path; needsRebuild = true; }
    void clear() { path.clear(); mesh.clear(); needsRebuild = true; }
    size_t size() const { return path.size(); }
    void rebuild();
};

// ============================================================================
// Surface Component
// ============================================================================

struct surface_component {
    int resolutionX, resolutionY;
    float width, height;
    std::vector<float> heightData;
    ofColor color;
    bool wireframe;
    bool smooth;
    ofMesh mesh;
    bool needsRebuild;
    
    surface_component()
        : resolutionX(20), resolutionY(20)
        , width(200), height(200)
        , color(255, 255, 255, 255)
        , wireframe(false)
        , smooth(true)
        , needsRebuild(true)
    {}
    
    void setResolution(int x, int y);
    void setHeight(int x, int y, float h) {
        if (x >= 0 && x < resolutionX && y >= 0 && y < resolutionY) {
            heightData[y * resolutionX + x] = h;
            needsRebuild = true;
        }
    }
    float getHeight(int x, int y) const {
        if (x >= 0 && x < resolutionX && y >= 0 && y < resolutionY) {
            return heightData[y * resolutionX + x];
        }
        return 0;
    }
    void generateFromFunction(std::function<float(float, float)> heightFunc);
    void generateWave(float amplitude = 20, float frequency = 2, float phase = 0);
    void generateNoise(float amplitude = 20, float scale = 1.0f);
    void rebuild();
};

// ============================================================================
// Visual Effect Components
// ============================================================================

struct outline_component {
    ofColor outlineColor;
    float outlineWidth;
    int passes;
    bool enabled;
    
    outline_component()
        : outlineColor(0, 0, 0, 255)
        , outlineWidth(2.0f)
        , passes(1)
        , enabled(true)
    {}
};

struct shadow_component {
    glm::vec2 offset;
    ofColor shadowColor;
    float blur;
    float intensity;
    bool enabled;
    
    shadow_component()
        : offset(5, 5)
        , shadowColor(0, 0, 0, 128)
        , blur(0)
        , intensity(0.5f)
        , enabled(true)
    {}
    
    ofColor getShadowColor() const { ofColor c = shadowColor; c.a *= intensity; return c; }
};

struct glow_component {
    ofColor glowColor;
    float intensity;
    float radius;
    int passes;
    bool enabled;
    
    glow_component()
        : glowColor(255, 255, 255, 255)
        , intensity(0.5f)
        , radius(10.0f)
        , passes(3)
        , enabled(true)
    {}
    
    ofColor getGlowColor(int pass) const {
        ofColor c = glowColor;
        c.a *= intensity * (1.0f - (float)pass / passes);
        return c;
    }
};

} // namespace ecs
