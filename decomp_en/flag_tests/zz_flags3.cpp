struct A { int v[4]; A(); };
struct B { int v[16]; B(); ~B(); };
struct C { int v[4]; C(); };
struct Thing { int head[4]; A a1; A a2; int gap[4]; B b; C c; Thing(); };
Thing::Thing() {}
