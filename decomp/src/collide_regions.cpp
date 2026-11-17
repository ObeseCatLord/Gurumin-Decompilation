// Collision against the flagged regions of a model: every node with flag 0x4 (visible) whose x/y bounds contain
// the position is tested in turn.

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
};

struct Node
{
    char pad_0000[0x6e4];
    Vec4 boundsMin;           // 0x6e4: x/y/z bounds of the node's mesh
    Vec4 boundsMax;           // 0x6f4
    Node *FindByFlags(unsigned int mask, int nth, int *counter);
    int Collide(int a, int b, Vec4 *position, Vec4 *velocity, Vec4 *hit);
};

// Moves `position` by `velocity` against the visible nodes of `root` whose x/y bounds contain the position
// (strictly inside). Stops at the first node that reports a collision (Node::Collide) and returns 1; 0 if none.
extern "C" int FUN_1001fff0(Node *root, int a, int b, Vec4 *position, Vec4 *velocity, Vec4 *hit)
{
    int i = 0;
    for (Node *node = root->FindByFlags(4, 0, 0); node != 0; node = root->FindByFlags(4, ++i, 0))
    {
        if (node->boundsMin.x < position->x && node->boundsMax.x > position->x && node->boundsMin.y < position->y &&
            node->boundsMax.y > position->y && node->Collide(a, b, position, velocity, hit) != 0)
            return 1;
    }
    return 0;
}
