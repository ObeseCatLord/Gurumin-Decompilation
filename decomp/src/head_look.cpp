// Head tracking of the player character: the head bone's parent node gets a saved-state block whose rotation limits
// bound how far the head may turn towards its target (Node::M_101948f0 enters the tracking state 0x400000).

struct Vec3
{
    float x;
    float y;
    float z;
};

// Saved-state block of a node (scene_node.cpp NodeSaved, 0xdc bytes).
struct NodeSaved
{
    char pad_0000[0x20];
    int state;                // 0x20: 2 = let the tracked rotation return to rest
    char pad_0024[0x54];
    Vec3 limitMin;            // 0x78: lowest rotation per axis (radians)
    int pad_0084;
    Vec3 limitMax;            // 0x88: highest rotation per axis
};

struct Node
{
    char pad_0000[0x74c];
    Node *parent;             // 0x74c
    char pad_0750[0x1104];
    NodeSaved *saved;         // 0x1854
    int M_101947b0();                 // allocates `saved` on first use
    int M_101948f0(Node *target);     // enters the tracking state (0 = default target)
    int M_10194980();                 // frees `saved`
};

extern "C" Node *DAT_10db91dc;  // the player's head node (character 0, field 0x2304)

// mode 1: starts head tracking, limited to +-1.22 rad around x, +-0.27 around y and +-0.17 around z;
// mode 0: lets the head return to rest; mode -5: removes the tracking block. Returns 0.
extern "C" int FUN_100165e0(int mode)
{
    if (DAT_10db91dc != 0)
    {
        if (mode == 1)
        {
            DAT_10db91dc->parent->M_101947b0();
            DAT_10db91dc->parent->saved->limitMin.x = -1.22f;
            DAT_10db91dc->parent->saved->limitMin.y = -0.27f;
            DAT_10db91dc->parent->saved->limitMin.z = -0.17f;
            DAT_10db91dc->parent->saved->limitMax.x = 1.22f;
            DAT_10db91dc->parent->saved->limitMax.y = 0.27f;
            DAT_10db91dc->parent->saved->limitMax.z = 0.17f;
            DAT_10db91dc->parent->M_101948f0(0);
            return 0;
        }
        if (mode == 0)
        {
            if (DAT_10db91dc->parent->saved != 0)
            {
                DAT_10db91dc->parent->saved->state = 2;
                return 0;
            }
        }
        else if (mode == -5 && DAT_10db91dc->parent->saved != 0)
            DAT_10db91dc->parent->M_10194980();
    }
    return 0;
}
