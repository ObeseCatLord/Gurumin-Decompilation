// Node::Init of the EN build. Ported from the matched JP source decomp/src/node_init.cpp.
#include "node.h"
#include <string.h>

// Node defaults set by Init.
const unsigned char NODE_DEFAULT_BRIGHTNESS = 30;
const unsigned char NODE_COLOUR_UNSET = 0xff;  // colourA / colourB before SetColourA / SetColourB
const unsigned char NODE_DEFAULT_187B = 45;    // m_187b after Init
const unsigned char NODE_DEFAULT_187E = 0xff;  // m_187e after Init
const unsigned char NODE_DEFAULT_1878 = 0xe0;  // m_1878 after Init
const float NODE_DEFAULT_1888 = 45.0f;         // m_1888 after Init
const float NODE_DEFAULT_CULL_MARGIN = 3.84f;  // cullMargin after Init
const float NODE_UNPLACED_X = 10000.0f;        // x translation of the world matrix before the node is placed
const int NODE_WORLD_TRANSLATION_X = 12;       // index of that translation in the matrix's 16 floats

// Node::Init: resets every field of a new node: called "root", no mesh, children or skin, identity world and local
// matrices, cull margin 3.84, unit scale (tint and fogColour) and the remaining defaults. Called when a node is created.
void Node::Init()
{
    savedFlags = 0;
    m_0088 = 0;
    isStatic = 0;
    staticCount = 0;
    staticNodes = 0;
    kind = NODE_KIND_NONE;
    m_0298.Fill(0);
    brightness = NODE_DEFAULT_BRIGHTNESS;
    colourA = NODE_COLOUR_UNSET;
    queueCallback = 0;
    drawCallback = 0;
    m_187a = 0;
    m_187b = NODE_DEFAULT_187B;
    pathKind = 0;
    m_187c = 0;
    m_187d = 0;
    m_187e = NODE_DEFAULT_187E;
    targetObject = g_defaultTargetObject;
    cycleIndex = 0;
    lightConeInner = 0;
    m_1888 = NODE_DEFAULT_1888;
    fogNear = 0;
    fogFar = 0;
    textureRef = 0;
    colourB = NODE_COLOUR_UNSET;
    element = 0;
    fogDensity = 1.0f;
    tint.Fill(1.0f);
    m_1878 = NODE_DEFAULT_1878;
    altPacks = 0;
    altIndexLists = 0;
    altPackCount = 0;
    mesh = 0;
    bounds.Set(0, 0, 0, 0);
    frameSource = 0;
    frame = 0;
    strcpy(name, "root");
    animated = 0;
    frameCount = 0;
    frameSize = 0;
    extra = 0;
    saved = 0;
    skin = 0;
    visibilityCells = 0;
    for (int i = 0; i < NODE_PLUG_COUNT; i++)
    {
        plugType[i] = 0;
        plugs[i] = 0;
        plugTypeCopy[i] = 0;
        plugData[i] = 0;
    }
    hasCollision = 0;
    model = 0;
    flags = 0;
    cullMargin = NODE_DEFAULT_CULL_MARGIN;
    group = 0;
    triangleSpheres = 0;
    m_1894 = 0;
    frameMatrixCount = 0;
    parent = 0;
    skinned = 0;
    materialCount = 0;
    materials = 0;
    childCount = 0;
    frameMatrices = 0;
    m_0060 = 0;
    packs = 0;
    indexLists = 0;
    packCount = 0;
    worldMatrix = IdentityMatrix();
    worldMatrix.m[NODE_WORLD_TRANSLATION_X] = NODE_UNPLACED_X;
    matrix = IdentityMatrix();
    parentName[0] = 0;
    memset(childNames, 0, sizeof(childNames));
    fogColour.x = 1.0f;
    fogColour.y = 1.0f;
    fogColour.z = 1.0f;
    fogColour.w = 1.0f;
}
