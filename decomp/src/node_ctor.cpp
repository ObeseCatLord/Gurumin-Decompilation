// Scene node constructor (node.h).

#include "matrix4.h"

#include "vec4.h"

#include "node.h"

// 0x1019ebc0. Member constructors run first (matrices and vectors), then Init (0x10191500) fills in the defaults.
// Used for the scene nodes embedded in WorldRenderer and allocated by the model loaders.
Node::Node()
{
    Init();
}
