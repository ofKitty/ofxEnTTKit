#include "rendering_components.h"
#include "ofLight.h"

namespace ecs {

// ============================================================================
// light_component
// ============================================================================

light_component::light_component()
    : light(std::make_unique<ofLight>())
    , type(POINT)
    , ambientColor(51, 51, 51, 255)
    , diffuseColor(255, 255, 255, 255)
    , specularColor(255, 255, 255, 255)
    , attenuation_constant(1.0f)
    , attenuation_linear(0.0f)
    , attenuation_quadratic(0.0f)
    , spotCutoff(45.0f)
    , spotConcentration(10.0f)
    , enabled(true)
    , castShadows(false)
    , drawDebug(false)
{
    light->enable();
}

light_component::~light_component() = default;

light_component::light_component(const light_component& other)
    : light(std::make_unique<ofLight>())
    , type(other.type)
    , ambientColor(other.ambientColor)
    , diffuseColor(other.diffuseColor)
    , specularColor(other.specularColor)
    , attenuation_constant(other.attenuation_constant)
    , attenuation_linear(other.attenuation_linear)
    , attenuation_quadratic(other.attenuation_quadratic)
    , spotCutoff(other.spotCutoff)
    , spotConcentration(other.spotConcentration)
    , enabled(other.enabled)
    , castShadows(other.castShadows)
    , drawDebug(other.drawDebug)
{}

light_component& light_component::operator=(const light_component& other) {
    if (this != &other) {
        type = other.type;
        ambientColor = other.ambientColor;
        diffuseColor = other.diffuseColor;
        specularColor = other.specularColor;
        attenuation_constant = other.attenuation_constant;
        attenuation_linear = other.attenuation_linear;
        attenuation_quadratic = other.attenuation_quadratic;
        spotCutoff = other.spotCutoff;
        spotConcentration = other.spotConcentration;
        enabled = other.enabled;
        castShadows = other.castShadows;
        drawDebug = other.drawDebug;
        light = std::make_unique<ofLight>();
    }
    return *this;
}

void light_component::apply() {
    if (enabled) {
        light->enable();
        light->setAmbientColor(ambientColor);
        light->setDiffuseColor(diffuseColor);
        light->setSpecularColor(specularColor);
        light->setAttenuation(attenuation_constant, attenuation_linear, attenuation_quadratic);

        if (type == SPOT) {
            light->setSpotlight();
            light->setSpotlightCutOff(spotCutoff);
            light->setSpotConcentration(spotConcentration);
        } else if (type == DIRECTIONAL) {
            light->setDirectional();
        } else if (type == POINT) {
            light->setPointLight();
        } else if (type == AREA) {
            light->setAreaLight(100, 100);
        }
    } else {
        light->disable();
    }
}

// ============================================================================
// material_component
// ============================================================================

void material_component::apply() {
    material.setAmbientColor(ambient);
    material.setDiffuseColor(diffuse);
    material.setSpecularColor(specular);
    material.setEmissiveColor(emissive);
    material.setShininess(shininess);
    material.setMetallic(metallic);
    material.setRoughness(roughness);
}

void material_component::begin() {
    if (type == MATERIAL_UNLIT) {
        ofSetColor(diffuse);
        return;
    }
    apply();
    material.begin();
}

void material_component::end() {
    if (type == MATERIAL_UNLIT) return;
    material.end();
}

void material_component::applyEnvironmentUniforms(ofShader& shader, const glm::vec3& cameraPos) {
    shader.setUniform3f("cameraPos", cameraPos);
    shader.setUniform1i("useRefraction", useRefraction ? 1 : 0);
    shader.setUniform1f("refractiveIndex", 1.0f / refractiveIndex);
    shader.setUniform1f("reflectivity", reflectivity);
    shader.setUniform1f("fresnelPower", fresnelPower);
}

// ============================================================================
// shader_component
// ============================================================================

bool shader_component::load(const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath) {
    vertPath = vertexPath;
    fragPath = fragmentPath;
    geomPath = geometryPath;
    
    if (geomPath.empty()) {
        isLoaded = shader.load(vertPath.string(), fragPath.string());
    } else {
        isLoaded = shader.load(vertPath.string(), fragPath.string(), geomPath.string());
    }
    
    if (isLoaded) {
        lastModifiedTime = std::filesystem::last_write_time(fragPath).time_since_epoch().count();
    }
    
    return isLoaded;
}

void shader_component::reload() {
    if (!vertPath.empty() && !fragPath.empty()) {
        load(vertPath.string(), fragPath.string(), geomPath.empty() ? "" : geomPath.string());
    }
}

void shader_component::checkForReload() {
    if (!autoReload || !isLoaded || fragPath.empty()) return;
    
    auto currentTime = std::filesystem::last_write_time(fragPath).time_since_epoch().count();
    if (currentTime != lastModifiedTime) {
        ofLogNotice("shader_component") << "Reloading shader: " << fragPath;
        reload();
        lastModifiedTime = currentTime;
    }
}

void shader_component::begin() {
    if (!isLoaded) return;
    
    shader.begin();
    
    for (auto& [name, value] : floatUniforms) {
        shader.setUniform1f(name, value);
    }
    for (auto& [name, value] : intUniforms) {
        shader.setUniform1i(name, value);
    }
    for (auto& [name, value] : vec2Uniforms) {
        shader.setUniform2f(name, value);
    }
    for (auto& [name, value] : vec3Uniforms) {
        shader.setUniform3f(name, value);
    }
    for (auto& [name, value] : vec4Uniforms) {
        shader.setUniform4f(name, value);
    }
    for (auto& [name, value] : colorUniforms) {
        shader.setUniform4f(name, value);
    }
}

void shader_component::end() {
    if (isLoaded) {
        shader.end();
    }
}

// ============================================================================
// primitive_component
// ============================================================================

void primitive_component::rebuild() {
    switch (type) {
        case BOX:
            primitive = ofBoxPrimitive(width, height, depth, resolution, resolution, resolution);
            break;
        case SPHERE:
            primitive = ofSpherePrimitive(radius, resolution);
            break;
        case CYLINDER:
            primitive = ofCylinderPrimitive(radiusTop, cylinderHeight, resolution, resolution);
            break;
        case CONE:
            primitive = ofConePrimitive(radiusBottom, cylinderHeight, resolution, resolution);
            break;
        case PLANE:
            primitive = ofPlanePrimitive(width, height, resolution, resolution);
            break;
        case ICOSPHERE:
            primitive = ofIcoSpherePrimitive(radius, resolution);
            break;
    }
    needsRebuild = false;
}

void primitive_component::drawWireframe() {
    if (needsRebuild) rebuild();
    primitive.drawWireframe();
}

// ============================================================================
// texture_component
// ============================================================================

bool texture_component::load(const std::string& path) {
    texturePath = path;
    ofImage img;
    if (img.load(path)) {
        texture = img.getTexture();
        applySettings();
        loaded = true;
        return true;
    }
    loaded = false;
    return false;
}

void texture_component::setTexture(const ofTexture& t) {
    texture = t;
    applySettings();
    loaded = texture.isAllocated();
}

void texture_component::applySettings() {
    if (!texture.isAllocated()) return;
    
    GLint wrapS = GL_REPEAT;
    GLint wrapT = GL_REPEAT;
    
    switch (wrapModeS) {
        case REPEAT: wrapS = GL_REPEAT; break;
        case MIRRORED_REPEAT: wrapS = GL_MIRRORED_REPEAT; break;
        case CLAMP_TO_EDGE: wrapS = GL_CLAMP_TO_EDGE; break;
        case CLAMP_TO_BORDER: wrapS = GL_CLAMP_TO_BORDER; break;
    }
    
    switch (wrapModeT) {
        case REPEAT: wrapT = GL_REPEAT; break;
        case MIRRORED_REPEAT: wrapT = GL_MIRRORED_REPEAT; break;
        case CLAMP_TO_EDGE: wrapT = GL_CLAMP_TO_EDGE; break;
        case CLAMP_TO_BORDER: wrapT = GL_CLAMP_TO_BORDER; break;
    }
    
    texture.setTextureWrap(wrapS, wrapT);
    
    GLint minFilter = GL_LINEAR;
    GLint magFilter = GL_LINEAR;
    
    switch (filterMode) {
        case NEAREST:
            minFilter = GL_NEAREST;
            magFilter = GL_NEAREST;
            break;
        case LINEAR:
            minFilter = GL_LINEAR;
            magFilter = GL_LINEAR;
            break;
        case LINEAR_MIPMAP_LINEAR:
            minFilter = GL_LINEAR_MIPMAP_LINEAR;
            magFilter = GL_LINEAR;
            texture.generateMipmap();
            break;
    }
    
    texture.setTextureMinMagFilter(minFilter, magFilter);
}

glm::mat3 texture_component::getUVMatrix() const {
    glm::mat3 mat(1.0f);
    
    mat = glm::mat3(
        1, 0, 0,
        0, 1, 0,
        offset.x, offset.y, 1
    ) * mat;
    
    float rad = glm::radians(rotation);
    float c = cos(rad);
    float s = sin(rad);
    mat = glm::mat3(
        c, -s, 0,
        s, c, 0,
        0.5f - 0.5f * c + 0.5f * s, 0.5f - 0.5f * s - 0.5f * c, 1
    ) * mat;
    
    mat = glm::mat3(
        tiling.x, 0, 0,
        0, tiling.y, 0,
        0, 0, 1
    ) * mat;
    
    return mat;
}

void texture_component::bind() {
    if (loaded && texture.isAllocated()) {
        texture.bind(textureUnit);
    }
}

void texture_component::unbind() {
    if (loaded && texture.isAllocated()) {
        texture.unbind(textureUnit);
    }
}

// ============================================================================
// cubemap_component
// ============================================================================

cubemap_component::~cubemap_component() {
    if (textureID != 0) {
        glDeleteTextures(1, &textureID);
        textureID = 0;
    }
}

bool cubemap_component::loadFromFaces(const std::string& posX, const std::string& negX,
                                       const std::string& posY, const std::string& negY,
                                       const std::string& posZ, const std::string& negZ) {
    std::vector<std::string> paths = {posX, negX, posY, negY, posZ, negZ};
    return loadFromFaces(paths);
}

bool cubemap_component::loadFromFaces(const std::vector<std::string>& paths) {
    if (paths.size() != 6) {
        ofLogError("cubemap_component") << "Need exactly 6 face paths";
        return false;
    }
    
    for (int i = 0; i < 6; i++) {
        facePaths[i] = paths[i];
    }
    
    useEquirectangular = false;
    
    if (textureID != 0) {
        glDeleteTextures(1, &textureID);
    }
    
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    
    bool allLoaded = true;
    for (int i = 0; i < 6; i++) {
        ofImage img;
        if (img.load(paths[i])) {
            img.setImageType(OF_IMAGE_COLOR);
            
            faceSize = img.getWidth();
            
            GLenum format = GL_RGB;
            if (img.getPixels().getNumChannels() == 4) {
                format = GL_RGBA;
            }
            
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGB,
                img.getWidth(), img.getHeight(), 0,
                format, GL_UNSIGNED_BYTE,
                img.getPixels().getData()
            );
            
            ofLogNotice("cubemap_component") << "Loaded face " << i << ": " << paths[i]
                << " (" << img.getWidth() << "x" << img.getHeight() << ")";
        } else {
            ofLogError("cubemap_component") << "Failed to load face " << i << ": " << paths[i];
            allLoaded = false;
        }
    }
    
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    
    loaded = allLoaded && (textureID != 0);
    return loaded;
}

bool cubemap_component::loadFromDirectory(const std::string& directory, const std::string& extension) {
    std::vector<std::string> faceNames = {"+x", "-x", "+y", "-y", "+z", "-z"};
    std::vector<std::string> paths;
    
    for (const auto& name : faceNames) {
        std::string path = directory + "/" + name + extension;
        if (!ofFile::doesFileExist(path)) {
            ofLogError("cubemap_component") << "Missing face: " << path;
            return false;
        }
        paths.push_back(path);
    }
    
    return loadFromFaces(paths);
}

bool cubemap_component::loadEquirectangular(const std::string& path) {
    equirectangularPath = path;
    useEquirectangular = true;
    
    ofImage img;
    if (img.load(path)) {
        equirectangularTexture = img.getTexture();
        loaded = true;
        return true;
    }
    
    loaded = false;
    return false;
}

void cubemap_component::bind(int textureUnit) {
    if (!loaded) return;
    
    if (useEquirectangular) {
        equirectangularTexture.bind(textureUnit);
    } else if (textureID != 0) {
        glActiveTexture(GL_TEXTURE0 + textureUnit);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    }
}

void cubemap_component::unbind(int textureUnit) {
    if (!loaded) return;
    
    if (useEquirectangular) {
        equirectangularTexture.unbind(textureUnit);
    } else {
        glActiveTexture(GL_TEXTURE0 + textureUnit);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }
}

// ============================================================================
// skybox_component
// ============================================================================

skybox_component::skybox_component() {
    skyboxMesh = createSkyboxMesh();
}

ofMesh skybox_component::createSkyboxMesh() {
    ofMesh mesh;
    mesh.setMode(OF_PRIMITIVE_TRIANGLES);
    
    float s = 1.0f;
    
    glm::vec3 vertices[] = {
        {-s, -s, -s}, { s, -s, -s}, { s,  s, -s}, {-s,  s, -s},
        { s, -s,  s}, {-s, -s,  s}, {-s,  s,  s}, { s,  s,  s},
        {-s, -s,  s}, {-s, -s, -s}, {-s,  s, -s}, {-s,  s,  s},
        { s, -s, -s}, { s, -s,  s}, { s,  s,  s}, { s,  s, -s},
        {-s,  s, -s}, { s,  s, -s}, { s,  s,  s}, {-s,  s,  s},
        {-s, -s,  s}, { s, -s,  s}, { s, -s, -s}, {-s, -s, -s}
    };
    
    for (int i = 0; i < 24; i++) {
        mesh.addVertex(vertices[i]);
    }
    
    for (int face = 0; face < 6; face++) {
        int base = face * 4;
        mesh.addIndex(base + 0);
        mesh.addIndex(base + 1);
        mesh.addIndex(base + 2);
        mesh.addIndex(base + 0);
        mesh.addIndex(base + 2);
        mesh.addIndex(base + 3);
    }
    
    return mesh;
}

// ============================================================================
// billboard_component
// ============================================================================

bool billboard_component::loadTexture(const std::string& path) {
    ofImage img;
    if (img.load(path)) {
        texture = img.getTexture();
        if (width <= 0) width = texture.getWidth();
        if (height <= 0) height = texture.getHeight();
        return true;
    }
    return false;
}

// Drawing handled by MeshRenderSystem::drawBillboard()

// ============================================================================
// trail_component
// ============================================================================

// Update and drawing handled by TrailSystem

// ============================================================================
// instanced_mesh_component
// ============================================================================

void instanced_mesh_component::addInstance(const glm::mat4& transform) {
    if (transforms.size() < maxInstances) {
        transforms.push_back(transform);
        if (useColors && colors.size() < transforms.size()) {
            colors.push_back(ofFloatColor(1, 1, 1, 1));
        }
        needsUpdate = true;
    }
}

void instanced_mesh_component::addInstance(const glm::vec3& position, const glm::quat& rotation, 
                                            const glm::vec3& scale) {
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
    transform = transform * glm::mat4_cast(rotation);
    transform = glm::scale(transform, scale);
    addInstance(transform);
}

void instanced_mesh_component::addInstance(const glm::vec3& position, const ofFloatColor& color) {
    addInstance(position);
    if (useColors && !colors.empty()) {
        colors.back() = color;
    }
}

void instanced_mesh_component::setInstanceTransform(size_t index, const glm::mat4& transform) {
    if (index < transforms.size()) {
        transforms[index] = transform;
        needsUpdate = true;
    }
}

void instanced_mesh_component::setInstanceColor(size_t index, const ofFloatColor& color) {
    if (index < colors.size()) {
        colors[index] = color;
        needsUpdate = true;
    }
}

// Drawing handled by MeshRenderSystem::drawInstancedMesh()

// ============================================================================
// tube_component
// ============================================================================

void tube_component::rebuild() {
    mesh.clear();
    
    if (path.size() < 2) {
        needsRebuild = false;
        return;
    }
    
    ofPolyline pathLine;
    for (const auto& p : path) {
        pathLine.addVertex(p);
    }
    
    if (smooth && pathResolution > 1) {
        pathLine = pathLine.getSmoothed(pathResolution);
    }
    
    std::vector<glm::vec3> smoothPath;
    for (size_t i = 0; i < pathLine.size(); i++) {
        smoothPath.push_back(pathLine[i]);
    }
    
    mesh.setMode(OF_PRIMITIVE_TRIANGLES);
    
    for (size_t i = 0; i < smoothPath.size(); i++) {
        glm::vec3 tangent;
        if (i == 0) {
            tangent = glm::normalize(smoothPath[1] - smoothPath[0]);
        } else if (i == smoothPath.size() - 1) {
            tangent = glm::normalize(smoothPath[i] - smoothPath[i-1]);
        } else {
            tangent = glm::normalize(smoothPath[i+1] - smoothPath[i-1]);
        }
        
        glm::vec3 up(0, 1, 0);
        if (std::abs(glm::dot(tangent, up)) > 0.99f) {
            up = glm::vec3(1, 0, 0);
        }
        glm::vec3 right = glm::normalize(glm::cross(up, tangent));
        up = glm::normalize(glm::cross(tangent, right));
        
        for (int j = 0; j < radialResolution; j++) {
            float angle = TWO_PI * j / radialResolution;
            glm::vec3 offset = (right * cos(angle) + up * sin(angle)) * radius;
            
            mesh.addVertex(smoothPath[i] + offset);
            mesh.addNormal(glm::normalize(offset));
            mesh.addColor(color);
        }
    }
    
    for (size_t i = 0; i < smoothPath.size() - 1; i++) {
        for (int j = 0; j < radialResolution; j++) {
            int current = i * radialResolution + j;
            int next = current + radialResolution;
            int currentNext = i * radialResolution + (j + 1) % radialResolution;
            int nextNext = currentNext + radialResolution;
            
            mesh.addIndex(current);
            mesh.addIndex(next);
            mesh.addIndex(currentNext);
            
            mesh.addIndex(currentNext);
            mesh.addIndex(next);
            mesh.addIndex(nextNext);
        }
    }
    
    if (caps && !closed) {
        int centerStart = mesh.getNumVertices();
        mesh.addVertex(smoothPath[0]);
        mesh.addNormal(-glm::normalize(smoothPath[1] - smoothPath[0]));
        mesh.addColor(color);
        
        for (int j = 0; j < radialResolution; j++) {
            int next = (j + 1) % radialResolution;
            mesh.addIndex(centerStart);
            mesh.addIndex(next);
            mesh.addIndex(j);
        }
        
        int lastRing = (smoothPath.size() - 1) * radialResolution;
        int centerEnd = mesh.getNumVertices();
        mesh.addVertex(smoothPath.back());
        mesh.addNormal(glm::normalize(smoothPath.back() - smoothPath[smoothPath.size()-2]));
        mesh.addColor(color);
        
        for (int j = 0; j < radialResolution; j++) {
            int next = (j + 1) % radialResolution;
            mesh.addIndex(centerEnd);
            mesh.addIndex(lastRing + j);
            mesh.addIndex(lastRing + next);
        }
    }
    
    needsRebuild = false;
}

// Drawing handled by MeshRenderSystem::drawTube()

// ============================================================================
// surface_component
// ============================================================================

void surface_component::setResolution(int x, int y) {
    resolutionX = x;
    resolutionY = y;
    heightData.resize(resolutionX * resolutionY, 0);
    needsRebuild = true;
}

void surface_component::generateFromFunction(std::function<float(float, float)> heightFunc) {
    heightData.resize(resolutionX * resolutionY);
    
    for (int y = 0; y < resolutionY; y++) {
        for (int x = 0; x < resolutionX; x++) {
            float u = (float)x / (resolutionX - 1);
            float v = (float)y / (resolutionY - 1);
            heightData[y * resolutionX + x] = heightFunc(u, v);
        }
    }
    needsRebuild = true;
}

void surface_component::generateWave(float amplitude, float frequency, float phase) {
    generateFromFunction([=](float u, float v) {
        return amplitude * sin(u * frequency * TWO_PI + phase) * 
               cos(v * frequency * TWO_PI + phase);
    });
}

void surface_component::generateNoise(float amplitude, float scale) {
    generateFromFunction([=](float u, float v) {
        return amplitude * ofNoise(u * scale, v * scale);
    });
}

void surface_component::rebuild() {
    mesh.clear();
    
    if (heightData.size() != resolutionX * resolutionY) {
        heightData.resize(resolutionX * resolutionY, 0);
    }
    
    mesh.setMode(OF_PRIMITIVE_TRIANGLES);
    
    float cellWidth = width / (resolutionX - 1);
    float cellHeight = height / (resolutionY - 1);
    
    for (int y = 0; y < resolutionY; y++) {
        for (int x = 0; x < resolutionX; x++) {
            float px = x * cellWidth - width / 2;
            float py = heightData[y * resolutionX + x];
            float pz = y * cellHeight - height / 2;
            
            mesh.addVertex(glm::vec3(px, py, pz));
            mesh.addTexCoord(glm::vec2((float)x / (resolutionX-1), 
                                       (float)y / (resolutionY-1)));
            mesh.addColor(color);
        }
    }
    
    for (int y = 0; y < resolutionY - 1; y++) {
        for (int x = 0; x < resolutionX - 1; x++) {
            int i00 = y * resolutionX + x;
            int i10 = i00 + 1;
            int i01 = i00 + resolutionX;
            int i11 = i01 + 1;
            
            mesh.addIndex(i00);
            mesh.addIndex(i01);
            mesh.addIndex(i10);
            
            mesh.addIndex(i10);
            mesh.addIndex(i01);
            mesh.addIndex(i11);
        }
    }
    
    if (smooth) {
        std::vector<glm::vec3> normals(mesh.getNumVertices(), glm::vec3(0));
        
        for (size_t i = 0; i < mesh.getNumIndices(); i += 3) {
            int i0 = mesh.getIndex(i);
            int i1 = mesh.getIndex(i + 1);
            int i2 = mesh.getIndex(i + 2);
            
            glm::vec3 v0 = mesh.getVertex(i0);
            glm::vec3 v1 = mesh.getVertex(i1);
            glm::vec3 v2 = mesh.getVertex(i2);
            
            glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
            
            normals[i0] += normal;
            normals[i1] += normal;
            normals[i2] += normal;
        }
        
        for (auto& n : normals) {
            n = glm::normalize(n);
        }
        
        for (const auto& n : normals) {
            mesh.addNormal(n);
        }
    }
    
    needsRebuild = false;
}

// ============================================================================
// outline_component, shadow_component, glow_component
// ============================================================================

// Drawing handled by EffectsRenderSystem

} // namespace ecs
