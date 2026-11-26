// The attachment slots of the kind-0x58 character (EN build; JP: decomp/include/attach_slots.h). Declared once here. Used by
// src/attach_slots.cpp.
#ifndef GURUMIN_EN_ATTACH_SLOTS_H
#define GURUMIN_EN_ATTACH_SLOTS_H

// One frame of the kind-0x58 character `index`: runs the state of each of its six attachment slots. Returns 0. C++ linkage
// (the original has an exception frame). EN 0x63b0a0 (JP 0x1001c670).
int Char_UpdateAttachSlots(int index);

#endif
