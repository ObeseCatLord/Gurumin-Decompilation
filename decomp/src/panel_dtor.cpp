// Destructor of a menu panel object that owns four resources (each with its own destructor).

#include "panel_dtor.h"


// Empty body: the compiler destroys the four Resource members in reverse order.
Panel::~Panel()
{
}
