#pragma once

#include "ofNode.h"

/// @brief Extended ofNode with ECS-friendly transform dirty tracking.
/// Uses a plain bool so the type remains movable (required for EnTT
/// swap_and_pop storage and view::size()).
class myNode : public ofNode {
public:
    myNode(glm::vec3 pos = {0,0,0}, std::string name = "");
    virtual ~myNode();
    
    const std::string& getName() const { return m_name; }
    void setName(const std::string& name) { m_name = name; }
    myNode* getParent();
    
    bool transformDirty{false};
    void clearDirty() { transformDirty = false; }
    
protected:
    std::string m_name;
    bool m_initialized{false};
    
    void onPositionChanged() override;
    void onOrientationChanged() override;
    void onScaleChanged() override;
};
