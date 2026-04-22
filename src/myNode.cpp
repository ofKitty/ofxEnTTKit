#include "myNode.h"

myNode::myNode(glm::vec3 pos /*= {0,0,0}*/, std::string name /*= ""*/)
    : m_name(name.empty() ? "Node" : name)
{
    setPosition(pos);
    m_initialized = true;  // Now safe to fire events
}

myNode::~myNode()
{
}

myNode* myNode::getParent()
{
    return dynamic_cast<myNode*>(ofNode::getParent());
}

// Transform change callbacks - called by ofNode setters
void myNode::onPositionChanged()
{
    transformDirty.store(true);
    if (m_initialized) {
        ofNotifyEvent(transformChanged, *this);
    }
}

void myNode::onOrientationChanged()
{
    transformDirty.store(true);
    if (m_initialized) {
        ofNotifyEvent(transformChanged, *this);
    }
}

void myNode::onScaleChanged()
{
    transformDirty.store(true);
    if (m_initialized) {
        ofNotifyEvent(transformChanged, *this);
    }
}
