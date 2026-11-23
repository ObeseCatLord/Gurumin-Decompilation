// MessageText: declared here once (first used by src/message_open.cpp).
#ifndef GURUMIN_MESSAGE_TEXT_H
#define GURUMIN_MESSAGE_TEXT_H

// The text of a message box (0x280 bytes).
struct MessageText
{
    char lines[4][0x80];
    char speaker[0x80];           // 0x200
};

// Number of message records (indexed by message slot, message_open.cpp).
#define MESSAGE_TEXT_COUNT 0x7c
// Bytes of the message records that Game_Reset clears: 0x10000, the first 102.4 of the 124 records (the size of
// an older table).
#define MESSAGE_TEXT_CLEARED_BYTES 0x10000

extern "C" MessageText DAT_10453208[MESSAGE_TEXT_COUNT];  // the text of each message box

#endif
