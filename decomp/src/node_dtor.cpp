// Destructor of the scene node body (0x18a0 bytes per node).

#include "matrix4.h"

#include "vec4.h"

#include "node.h"


// 0x10191480. Empty body: the compiler destroys the matrix members and the array of sixteen matrices.
Node::~Node()
{
}

