// Per-character effect spawning: the effect manager (DAT_10daaa80, EffectManager) starts an effect and writes
// its handle into the character's entry of DAT_10219770.

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
};

struct EffectManager
{
    // Starts effect `effect` at `position`; the seven values are its options (colour, follow target, flags, ...;
    // callers mostly pass -1 and 0). Stores the effect handle in *handle.
    int M_10172350(int effect, Vec4 position, int a, int b, int c, int d, int e, int f, int g, int *handle);
};

extern "C" EffectManager DAT_10daaa80;
extern "C" int DAT_10219770[0x80];  // effect handle of each character slot

// Starts effect `effect` at `position` for character slot `index` and records its handle in that slot.
extern "C" int FUN_1001d8b0(int index, int effect, Vec4 position, int a, int b, int c, int d, int e, int f, int g)
{
    DAT_10daaa80.M_10172350(effect, position, a, b, c, d, e, f, g, &DAT_10219770[index]);
    return 0;
}
