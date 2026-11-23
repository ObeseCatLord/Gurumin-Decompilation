// RoutePoint: declared here once (first used by src/map_route.cpp).
#ifndef GURUMIN_ROUTE_POINT_H
#define GURUMIN_ROUTE_POINT_H

// A route point: a position and a code. The code's low four digits (as a signed value, 5001..9999 meaning
// -4999..-1) are an extra cost of the point; the digits above them (signed the same way) modulo 4 above 1 end the
// route.
struct RoutePoint
{
    int x;
    int y;
    int z;
    int code;
};

#endif
