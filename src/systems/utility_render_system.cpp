#include "utility_render_system.h"

namespace ecs {
using namespace ecs;

void UtilityRenderSystem::draw(entt::registry& registry) {
    // Draw grid helpers
    auto grids = registry.view<grid_helper_component, node_component>();
    for (auto entity : grids) {
		auto & cGrid = registry.get<grid_helper_component>(entity);
		auto & cNode = registry.get<node_component>(entity);
        
        ofPushMatrix();
        ofMultMatrix(cNode.node.getGlobalTransformMatrix());
        drawGridHelper(cGrid);
        ofPopMatrix();
    }
   
    // Draw bounding boxes
    auto boxes = registry.view<bounding_box_component, node_component>();
    for (auto entity : boxes) {
        auto& cBox = registry.get<bounding_box_component>(entity);
        auto& cNode = registry.get<node_component>(entity);
        
        ofPushMatrix();
        ofMultMatrix(cNode.node.getGlobalTransformMatrix());
        drawBoundingBox(cBox);
        ofPopMatrix();
    }
}

void UtilityRenderSystem::drawGridHelper(const grid_helper_component& comp) {
    ofPushStyle();
    ofSetLineWidth(1);

    float halfSize = comp.size / 2.0f;
    float step = comp.size / comp.divisions;

    // Draw XZ plane (ground)
    if (comp.showXZ) {
        for (int i = 0; i <= comp.divisions; i++) {
            float pos = -halfSize + i * step;
            
            if (i == comp.divisions / 2) {
                ofSetColor(comp.centerLineColor);
            } else {
                ofSetColor(comp.gridColor);
            }
            
            ofDrawLine(-halfSize, 0, pos, halfSize, 0, pos);
            ofDrawLine(pos, 0, -halfSize, pos, 0, halfSize);
        }
    }
    
    // Draw XY plane
    if (comp.showXY) {
        for (int i = 0; i <= comp.divisions; i++) {
            float pos = -halfSize + i * step;
            
            if (i == comp.divisions / 2) {
                ofSetColor(comp.centerLineColor);
            } else {
                ofSetColor(comp.gridColor);
            }
            
            ofDrawLine(-halfSize, pos, 0, halfSize, pos, 0);
            ofDrawLine(pos, -halfSize, 0, pos, halfSize, 0);
        }
    }
    
    // Draw YZ plane
    if (comp.showYZ) {
        for (int i = 0; i <= comp.divisions; i++) {
            float pos = -halfSize + i * step;
            
            if (i == comp.divisions / 2) {
                ofSetColor(comp.centerLineColor);
            } else {
                ofSetColor(comp.gridColor);
            }
            
            ofDrawLine(0, -halfSize, pos, 0, halfSize, pos);
            ofDrawLine(0, pos, -halfSize, 0, pos, halfSize);
        }
    }
    
    // Draw axes
    if (comp.showAxes) {
        ofSetLineWidth(3);
        
        ofSetColor(255, 0, 0);
        ofDrawLine(0, 0, 0, comp.axisLength, 0, 0);
        
        ofSetColor(0, 255, 0);
        ofDrawLine(0, 0, 0, 0, comp.axisLength, 0);
        
        ofSetColor(0, 0, 255);
        ofDrawLine(0, 0, 0, 0, 0, comp.axisLength);
    }
    
    ofPopStyle();
}

void UtilityRenderSystem::drawBoundingBox(const bounding_box_component & comp) {
    if (!comp.visible) return;
    
    ofPushStyle();
    ofNoFill();
    ofSetColor(comp.color);
    ofSetLineWidth(2);
    
    glm::vec3 size = comp.getSize();
    glm::vec3 center = comp.getCenter();
    
    ofPushMatrix();
    ofTranslate(center);
    ofDrawBox(size.x, size.y, size.z);
    ofPopMatrix();
    
    ofPopStyle();
}

} // namespace ecs
