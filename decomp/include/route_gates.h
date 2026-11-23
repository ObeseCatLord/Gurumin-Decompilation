// Route_FollowGates: the walker of the routes whose points carry gates (route_follow.cpp).
#ifndef GURUMIN_ROUTE_GATES_H
#define GURUMIN_ROUTE_GATES_H

// 0x1002f050 (Route_FollowGates), see route_follow.cpp.
extern "C" int FUN_1002f050(int walker, int route, int point, int *outPoint, float *position, int *outType,
                            int *outParam, int *outValue);

#endif
