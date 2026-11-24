// File access of the sound engine (EN build; JP twin: sound3d.dll, not matched yet). The MmioFile methods are inline in
// sound_file.h because the original inlines them into the sound code. The original also has a standalone copy of
// Close (0x7bf150), Read (0x7bf540), Seek (0x7bf5d0) and Tell (0x7bf630), emitted where some sound function calls them
// out of line; they are compared once those callers (the large unported open/decode functions) are written.
// No address-taking table here: data the original does not have would only exist to steer the comparison.
#include "sound_file.h"
