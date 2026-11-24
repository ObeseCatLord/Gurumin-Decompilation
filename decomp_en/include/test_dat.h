// The word groups read from test.dat (EN build). This is the one declaration of these tables in decomp_en.
#ifndef GURUMIN_EN_TEST_DAT_H
#define GURUMIN_EN_TEST_DAT_H

// Most groups, words per group and characters per word (with the terminator) test.dat can hold.
const int TEST_DAT_GROUP_COUNT = 128;
const int TEST_DAT_WORDS_PER_GROUP = 0x100;
const int TEST_DAT_WORD_SIZE = 0x20;

// Number of word groups read. EN 0x00b48fb0 (JP DAT_104c02b0).
extern "C" int g_testGroupCount;
// Number of words in each group. EN 0x00b48db0 (JP DAT_103084a8).
extern "C" int g_testGroupWordCount[TEST_DAT_GROUP_COUNT];
// Replacement chosen for each word (0xff = not yet). EN 0x00b40db0 (JP DAT_1030ceb8).
extern "C" unsigned char g_testWordChoice[TEST_DAT_GROUP_COUNT][TEST_DAT_WORDS_PER_GROUP];
// The words. EN 0x00a38da8 (JP DAT_1032d870).
extern "C" char g_testWords[TEST_DAT_GROUP_COUNT][TEST_DAT_WORDS_PER_GROUP][TEST_DAT_WORD_SIZE];
// Length of each word. EN 0x00b38da8 (JP DAT_104487e8).
extern "C" char g_testWordLength[TEST_DAT_GROUP_COUNT][TEST_DAT_WORDS_PER_GROUP];

// Reads test.dat from the data folder into the word groups. EN 0x609c90 (JP TestDat_Load).
extern "C" int TestDat_Load(void);

#endif
