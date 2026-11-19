#include <math.h>
extern "C" float g_a, g_b, g_out, g_out2;
extern "C" void use(float);
inline float AbsF(double v) { return (float)fabs(v); }
inline float AbsLocal(double v) { float r = (float)fabs(v); return r; }
extern "C" void p1() { float h = (float)fabs(g_a - g_b); use(h); g_out = h; }
extern "C" void p2() { float h; h = (float)fabs(g_a - g_b); g_out2 = g_a; use(h); }
extern "C" void p3() { volatile float h = (float)fabs(g_a - g_b); use(h); }
extern "C" void p4() { float h = AbsLocal(g_a - g_b); use(h); use(h); }
extern "C" void p5() { double d = fabs(g_a - g_b); float h = (float)d; use(h); use(h); }
extern "C" void p6() { float h = (float)fabs(g_a - g_b); float k = h; use(k); use(h); }
extern "C" void p7() { float h = (float)(float)fabs(g_a - g_b); use(h); use(h);}
extern "C" void p8() { float t = (float)fabs(g_a - g_b); float h = t + 0.0f; use(h); use(h);}
struct V2 { float x; float y; };
extern "C" void p9() { V2 d; d.y = (float)fabs(g_a - g_b); d.x = (float)fabs(g_b - g_a); use(d.x + d.y); }
extern "C" void p10() { float d[2]; d[1] = (float)fabs(g_a - g_b); d[0] = (float)fabs(g_b - g_a); use(d[0] + d[1]); }
extern "C" void p11() { float h = AbsF(g_a - g_b); float s = AbsF(g_b - g_a) + h; use(s); use(h); }
extern "C" void p12() { float s = (float)fabs(g_a - g_b) + (float)fabs(g_b - g_a); use(s); }
