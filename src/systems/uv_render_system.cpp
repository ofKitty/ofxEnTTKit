#include "uv_render_system.h"
#include "mesh_render_system.h"

namespace ecs {

void UVRenderSystem::draw(entt::registry& registry) {
    auto view = registry.view<uv_component, node_component>();
    for (auto entity : view) {
        auto& uv = view.get<uv_component>(entity);
        if (!uv.showPreview) continue;
        auto& nc = view.get<node_component>(entity);
        ofPushMatrix();
        ofMultMatrix(nc.node.getGlobalTransformMatrix());
        drawEntity(registry, entity);
        ofPopMatrix();
    }
}

void UVRenderSystem::drawEntity(entt::registry& registry, entt::entity entity) {
    if (!registry.all_of<uv_component>(entity)) return;
    auto& uv = registry.get<uv_component>(entity);
    if (!uv.showPreview) return;

    const size_t n = uv.pixels.size();
    for (size_t i = 0; i < n; ++i) {
        const auto& pixel = uv.pixels[i];
        if (pixel.index >= 0 && pixel.index < static_cast<int>(uv.sampledColors.size())) {
            ofSetColor(uv.sampledColors[pixel.index]);
        } else {
            ofSetColor(pixel.color);
        }
        float x = pixel.uv.x * uv.width;
        float y = pixel.uv.y * uv.height;

        if (uv.drawableEntity != entt::null && registry.valid(uv.drawableEntity)) {
            drawDrawableAt(registry, uv.drawableEntity, x, y, uv.drawScale);
        } else {
            ofDrawCircle(x, y, 5.0f * uv.drawScale);
        }
    }
}

void UVRenderSystem::drawDrawableAt(entt::registry& registry, entt::entity drawableEntity, float x, float y, float scale) {
    ofPushMatrix();
    ofTranslate(x, y, 0);
    ofScale(scale);

    if (registry.any_of<mesh_component>(drawableEntity)) {
        auto& mc = registry.get<mesh_component>(drawableEntity);
        ofPushStyle();
        ofSetColor(mc.color);
        if (mc.drawFaces) mc.m_mesh.draw();
        if (mc.drawWireframe) mc.m_mesh.drawWireframe();
        ofPopStyle();
    } else if (registry.any_of<image_component>(drawableEntity)) {
        auto& ic = registry.get<image_component>(drawableEntity);
        if (ic.image.isAllocated()) {
            ofPushStyle();
            ofEnableAlphaBlending();
            ofSetColor(255);
            float w = ic.image.getWidth();
            float h = ic.image.getHeight();
            ic.image.draw(-w * 0.5f, -h * 0.5f, w, h);
            ofPopStyle();
        }
    } else {
        ofDrawCircle(0, 0, 5.0f);
    }

    ofPopMatrix();
}

} // namespace ecs
