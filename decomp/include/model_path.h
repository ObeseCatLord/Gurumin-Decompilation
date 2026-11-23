// ModelPath: declared here once (first used by src/char_motion.cpp).
#ifndef GURUMIN_MODEL_PATH_H
#define GURUMIN_MODEL_PATH_H

// Model path table at 0x10b314c8: one 0x4b34-byte record per model id, starting with the file name.
struct ModelPath
{
    char path[0x4b34];
};

#endif
