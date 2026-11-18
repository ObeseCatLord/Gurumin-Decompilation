// Material colours of a node's mesh parts. A node (scene_node.cpp Node) has `packCount` packs; each pack has
// `partCount` parts of 0x70 bytes, and each part names its material by number.

struct Colour
{
    float r;
    float g;
    float b;
    float a;
};

// One mesh part, 0x70 bytes.
struct MeshPart
{
    char pad_0000[0x20];
    Colour colour;            // 0x20: diffuse colour taken from the material
    char pad_0030[0xc];
    float material;           // 0x3c: material number (stored as a float)
    Colour emissive;          // 0x40: reset to (0, 0, 0, 1)
    char pad_0050[0x20];
};

// One pack of parts, 0x74 bytes (scene_node.cpp PackHeader).
struct MeshPack
{
    char pad_0000[0x1c];
    int partCount;            // 0x1c
    char pad_0020[0x50];
    MeshPart *parts;          // 0x70
};

// Material, 0x180 bytes (scene_node.cpp Material).
struct MeshMaterial
{
    char pad_0000[0x140];
    float colour[3];          // 0x140: r, g, b
    char pad_014c[0x2c];
    float alpha;              // 0x178: used when positive
    char pad_017c[4];
};

struct Node
{
    char pad_0000[0x64];
    int packCount;            // 0x64
    MeshPack *packs;          // 0x68
    char pad_006c[0x28];
    MeshMaterial *materials;  // 0x94
    int ApplyMaterialColours();
};

// 0x1019c560. Copies each part's material colour into the part (r, g, b from the material, alpha from the
// material when positive, else 0) and resets the part's second colour to (0, 0, 0, 1). Returns 0.
int Node::ApplyMaterialColours()
{
    for (int i = 0; i < packCount; i++)
    {
        for (int j = 0; j < packs[i].partCount; j++)
        {
            MeshMaterial *material = &materials[(int)packs[i].parts[j].material];
            float r = material->colour[0];
            float g = material->colour[1];
            float b = material->colour[2];
            float a = 0.0f < material->alpha ? material->alpha : 0;
            packs[i].parts[j].colour.r = r;
            packs[i].parts[j].colour.g = g;
            packs[i].parts[j].colour.b = b;
            packs[i].parts[j].colour.a = a;
            packs[i].parts[j].emissive.r = 0;
            packs[i].parts[j].emissive.g = 0;
            packs[i].parts[j].emissive.b = 0;
            packs[i].parts[j].emissive.a = 1.0f;
        }
    }
    return 0;
}
