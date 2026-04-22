#pragma once

#include "ofNode.h"
#include "ofEvents.h"
#include <atomic>

/// @brief Extended ofNode with transform change events
/// Suitable for use in ECS systems (ofxEnTT compatible)
/// Thread-safe dirty flag for use with OF callbacks/listeners
class myNode : public ofNode {
public:
    myNode(glm::vec3 pos = {0,0,0}, std::string name = "");
    virtual ~myNode();
    
    const std::string& getName() const { return m_name; }
    void setName(const std::string& name) { m_name = name; }
    myNode* getParent();
    
    // Transform change detection (thread-safe)
    std::atomic<bool> transformDirty{false};
    void clearDirty() { transformDirty.store(false); }
    
    // Event fired when any transform property changes
    ofEvent<myNode&> transformChanged;
    
protected:
    std::string m_name;
    bool m_initialized{false};  // Prevent events during construction
    
    // Override ofNode virtual callbacks
    void onPositionChanged() override;
    void onOrientationChanged() override;
    void onScaleChanged() override;
};
