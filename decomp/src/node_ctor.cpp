// Scene node constructor.

#include "matrix4.h"

#include "vec4.h"

#include "scene_node.h"

// Member constructors run first (matrices and vectors), then Init (0x10191500) fills in the defaults.
SceneNode::SceneNode()
{
    M_10191500();
}
