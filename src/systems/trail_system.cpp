#include "trail_system.h"

namespace ecs {
using namespace ecs;

void TrailSystem::update(entt::registry& registry, float deltaTime) {
	auto view = registry.view<trail_component, node_component>();
    
    for (auto entity : view) {
		auto & trail = registry.get<trail_component>(entity);
		auto & node = registry.get<node_component>(entity);
        
        // Optionally auto-add points based on node position
        if (trail.autoAddPoints) {
            addTrailPoint(trail, node.node.getGlobalPosition());
        }
        
        updateTrail(trail, deltaTime);
    }
}

void TrailSystem::draw(entt::registry& registry) {
    if (!m_camera) return;
    
    auto view = registry.view<trail_component>();
    
    for (auto entity : view) {
		auto & trail = registry.get<trail_component>(entity);
        drawTrailRibbon(trail, *m_camera);
    }
}

void TrailSystem::addTrailPoint(trail_component & comp, const glm::vec3 & position) {
	trail_component::TrailPoint point;
    point.position = position;
    point.age = 0;
    point.color = comp.colorStart;
    point.width = comp.widthStart;
    
    comp.points.push_back(point);
    
    while (comp.points.size() > comp.maxPoints) {
        comp.points.erase(comp.points.begin());
    }
}

void TrailSystem::updateTrail(trail_component & comp, float dt) {
    for (auto& p : comp.points) {
        p.age += dt;
        float t = p.age / comp.lifetime;
        t = ofClamp(t, 0.0f, 1.0f);
        p.color = comp.colorStart.getLerped(comp.colorEnd, t);
        p.width = ofLerp(comp.widthStart, comp.widthEnd, t);
    }
    
    // Remove dead points
    comp.points.erase(
        std::remove_if(comp.points.begin(), comp.points.end(), 
            [&comp](const trail_component::TrailPoint & p) { return p.age >= comp.lifetime; }),
        comp.points.end()
    );
}

void TrailSystem::drawTrailRibbon(const trail_component & comp, const ofCamera & camera) {
    if (comp.points.size() < 2) return;
    
    ofMesh mesh;
    mesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
    
    glm::vec3 camPos = camera.getPosition();
    
    for (size_t i = 0; i < comp.points.size(); i++) {
        const auto& p = comp.points[i];
        
        glm::vec3 toCamera = glm::normalize(camPos - p.position);
        
        glm::vec3 tangent;
        if (i < comp.points.size() - 1) {
            tangent = glm::normalize(comp.points[i+1].position - p.position);
        } else {
            tangent = glm::normalize(p.position - comp.points[i-1].position);
        }
        
        glm::vec3 perpendicular = glm::normalize(glm::cross(tangent, toCamera));
        
        mesh.addVertex(p.position + perpendicular * p.width * 0.5f);
        mesh.addColor(p.color);
        mesh.addVertex(p.position - perpendicular * p.width * 0.5f);
        mesh.addColor(p.color);
    }
    
    mesh.draw();
}

void TrailSystem::clearTrail(trail_component & comp) {
    comp.points.clear();
}

} // namespace ecs
