#include "gizmo_system.h"

namespace ecs {
using namespace ecs;

void GizmoSystem::draw(entt::registry& registry) {
	auto view = registry.view<gizmo_component, node_component>();
    
    for (auto entity : view) {
		auto & cGizmo = registry.get<gizmo_component>(entity);
		auto & cNode = registry.get<node_component>(entity);
        
        if (cGizmo.enabled) {
            drawGizmo(cGizmo, cNode.node.getGlobalPosition());
        }
    }
}

void GizmoSystem::drawGizmo(const gizmo_component & comp, const glm::vec3 & position) {
    if (!comp.enabled) return;
    
    ofPushStyle();
    ofPushMatrix();
    ofTranslate(position);
    
    switch (comp.mode) {
        case gizmo_component::TRANSLATE:
            drawTranslateGizmo(comp);
            break;
		case gizmo_component::ROTATE:
            drawRotateGizmo(comp);
            break;
		case gizmo_component::SCALE:
            drawScaleGizmo(comp);
            break;
    }
    
    ofPopMatrix();
    ofPopStyle();
}

void GizmoSystem::drawTranslateGizmo(const gizmo_component& comp) {
    float arrowLength = comp.size;
    float arrowHead = comp.size * 0.2f;
    
    ofSetLineWidth(3);
    
    // X axis
	ofSetColor(comp.selectedAxis == gizmo_component::X ? comp.selectedColor : comp.xColor);
    ofDrawLine(0, 0, 0, arrowLength, 0, 0);
    ofDrawCone(arrowLength, 0, 0, arrowHead, arrowHead * 1.5f);
    
    // Y axis
	ofSetColor(comp.selectedAxis == gizmo_component::Y ? comp.selectedColor : comp.yColor);
    ofDrawLine(0, 0, 0, 0, arrowLength, 0);
    ofPushMatrix();
    ofTranslate(0, arrowLength, 0);
    ofRotateDeg(90, 1, 0, 0);
    ofDrawCone(arrowHead, arrowHead * 1.5f);
    ofPopMatrix();
    
    // Z axis
	ofSetColor(comp.selectedAxis == gizmo_component::Z ? comp.selectedColor : comp.zColor);
    ofDrawLine(0, 0, 0, 0, 0, arrowLength);
    ofPushMatrix();
    ofTranslate(0, 0, arrowLength);
    ofRotateDeg(-90, 0, 1, 0);
    ofDrawCone(arrowHead, arrowHead * 1.5f);
    ofPopMatrix();
}

void GizmoSystem::drawRotateGizmo(const gizmo_component & comp) {
    float radius = comp.size;
    
    ofNoFill();
    ofSetLineWidth(3);
    
    // X axis (YZ plane)
	ofSetColor(comp.selectedAxis == gizmo_component::X ? comp.selectedColor : comp.xColor);
    ofPushMatrix();
    ofRotateDeg(90, 0, 1, 0);
    ofDrawCircle(0, 0, radius);
    ofPopMatrix();
    
    // Y axis (XZ plane)
	ofSetColor(comp.selectedAxis == gizmo_component::Y ? comp.selectedColor : comp.yColor);
    ofPushMatrix();
    ofRotateDeg(90, 1, 0, 0);
    ofDrawCircle(0, 0, radius);
    ofPopMatrix();
    
    // Z axis (XY plane)
	ofSetColor(comp.selectedAxis == gizmo_component::Z ? comp.selectedColor : comp.zColor);
    ofDrawCircle(0, 0, radius);
}

void GizmoSystem::drawScaleGizmo(const gizmo_component & comp) {
    float length = comp.size;
    float cubeSize = comp.size * 0.15f;
    
    ofSetLineWidth(3);
    
    // X axis
	ofSetColor(comp.selectedAxis == gizmo_component::X ? comp.selectedColor : comp.xColor);
    ofDrawLine(0, 0, 0, length, 0, 0);
    ofPushMatrix();
    ofTranslate(length, 0, 0);
    ofDrawBox(cubeSize, cubeSize, cubeSize);
    ofPopMatrix();
    
    // Y axis
	ofSetColor(comp.selectedAxis == gizmo_component::Y ? comp.selectedColor : comp.yColor);
    ofDrawLine(0, 0, 0, 0, length, 0);
    ofPushMatrix();
    ofTranslate(0, length, 0);
    ofDrawBox(cubeSize, cubeSize, cubeSize);
    ofPopMatrix();
    
    // Z axis
	ofSetColor(comp.selectedAxis == gizmo_component::Z ? comp.selectedColor : comp.zColor);
    ofDrawLine(0, 0, 0, 0, 0, length);
    ofPushMatrix();
    ofTranslate(0, 0, length);
    ofDrawBox(cubeSize, cubeSize, cubeSize);
    ofPopMatrix();
    
    // Center (all axes)
	ofSetColor(comp.selectedAxis == gizmo_component::XYZ ? comp.selectedColor : ofColor(150));
    ofDrawBox(cubeSize * 0.8f, cubeSize * 0.8f, cubeSize * 0.8f);
}

} // namespace ecs
