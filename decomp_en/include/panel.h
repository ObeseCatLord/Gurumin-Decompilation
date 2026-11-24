// The menu panel object of the EN build (JP: decomp/include/panel_dtor.h). Only the members EN code has used so far
// are named. This is the one declaration of Panel in decomp_en: include this header, never redeclare it.
#ifndef GURUMIN_EN_PANEL_H
#define GURUMIN_EN_PANEL_H

#include "vec4.h"

struct Panel
{
    char m_0000[0x18];     // 0x00: not used by EN code yet
    Matrix4 m_0018;        // 0x18: first member with an empty destructor
    char m_0058[0x1d8 - 0x58];       // 0x58: not used by EN code yet
    Matrix4 m_01d8;        // 0x1d8: second member
    char m_0218[0x2328 - 0x218];     // 0x218: not used by EN code yet
    Matrix4 m_2328;        // 0x2328: third member
    char m_2368[0x2434 - 0x2368];    // 0x2368: not used by EN code yet
    Matrix4 m_2434;        // 0x2434: fourth member

    ~Panel();              // EN 0x5a10d0 (JP 0x10007e10): destroys the four members
};

#endif
