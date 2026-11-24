// Small text helpers shared by the script and message code (EN build).
#ifndef GURUMIN_EN_TEXT_SUBSTITUTE_H
#define GURUMIN_EN_TEXT_SUBSTITUTE_H

// A message that starts with this character is a label reference: the character itself is not copied.
const char TEXT_LABEL_MARKER = '#';

// Copies `text` to `out`, leaving out a leading '#' marker. Always returns 1. EN 0x5c9510 (JP 0x1000c020).
extern "C" int Text_CopyStripMarker(char *out, const char *text);

#endif
