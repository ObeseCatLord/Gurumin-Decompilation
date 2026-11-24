// Globals and helpers the model file loader (Node::ParseChunks, model_parse_chunks.cpp) uses to give materials their
// textures. This is the one declaration of these globals: include this header, never redeclare them.
#ifndef GURUMIN_MODEL_LOAD_VARS_H
#define GURUMIN_MODEL_LOAD_VARS_H

#include "texture_object.h"

// Name of the fur texture being loaded; "Fur-<name>" is the copy a material falls back to when its texture is missing.
extern "C" char DAT_10ffe5b8[];
// The texture object copied as the fur fallback texture (FUN_10175a90).
extern "C" TextureObject DAT_118d0b70;
// Pixels of the blank "auto-white" texture (128 x 128).
extern "C" unsigned char DAT_11af3fa8[];
// 0x10175c90: creates the texture `name` from `pixels` (all white when 0). Returns its slot.
// 0x10175a90 (Texture_CreateCopy): the texture `name` with the size and pixels of `source`.
extern "C" int FUN_10175a90(const char *name, const TextureObject *source, int flags);

#endif
