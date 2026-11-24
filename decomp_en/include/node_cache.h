// The cache of loaded model files (EN build; JP: decomp/src/node_cache.cpp). Node::Load keeps every model file it loaded
// as a node tree here and copies from it. This is the one declaration of these tables.
#ifndef GURUMIN_EN_NODE_CACHE_H
#define GURUMIN_EN_NODE_CACHE_H

#include "node.h"

// Cache slots, and the bytes of a cached file's name.
const int NODE_CACHE_SIZE = 0x80;
const int NODE_CACHE_NAME_SIZE = 0x80;

extern "C" Node *g_cachedNodes[NODE_CACHE_SIZE];                           // EN 0x0217b8e0 (JP DAT_11b002d0): the loaded model tree of each slot (0 = free)
extern "C" char g_cachedNodeNames[NODE_CACHE_SIZE][NODE_CACHE_NAME_SIZE];  // EN 0x0217bae0 (JP DAT_11b004d0): file name of each cached model

#endif
