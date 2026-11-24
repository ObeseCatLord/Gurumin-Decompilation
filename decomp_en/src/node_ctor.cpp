// Scene node constructor (JP twin: node_ctor.cpp).

#include "node.h"

// Member constructors run first (matrices and vectors), then Init fills in the defaults. Used for the scene nodes
// embedded in WorldRenderer and allocated by the model loaders.
Node::Node()
{
    Init();
}
