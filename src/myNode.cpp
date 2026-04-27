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

void myNode::onPositionChanged()    { transformDirty = true; }
void myNode::onOrientationChanged() { transformDirty = true; }
void myNode::onScaleChanged()       { transformDirty = true; }
