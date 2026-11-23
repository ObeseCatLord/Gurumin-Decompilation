// MessageText: declared here once (first used by src/message_open.cpp).
#ifndef GURUMIN_MESSAGE_TEXT_H
#define GURUMIN_MESSAGE_TEXT_H

// The text of a message box (0x280 bytes).
struct MessageText
{
    char lines[4][0x80];
    char speaker[0x80];           // 0x200
};

#endif
