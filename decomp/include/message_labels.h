#pragma once
// Message box labels (message_labels.cpp).

// Set to 1 while the message box labels are drawn, 0 after (only written in the shipped game).
extern "C" int DAT_102108cc;

struct Matrix4;

// Message_DrawLabels: draws the label panel, speaker name and text lines of each open message box with the
// view matrix; returns 0 when the labels are hidden.
extern "C" int FUN_1000cf80(Matrix4 *view);
