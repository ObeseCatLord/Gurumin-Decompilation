#pragma once

// Event trigger (0xc8 bytes): runs when its map matches and all four script flags pass.
struct GameTrigger
{
    int map;                    // map number, 9999 = any map
    int flags[4];               // script flags that must be set (negative = must be clear, 0 = no condition)
    int value;                  // 0x14: event id (for map events)
    char name[0x10];            // 0x18: event name, e.g. "GAMEOVER", "KSKON", "TALK", "OPEN"
    int argument;               // 0x28: what the event reacts to (for example the character or object id)
    char pad_002c[0x9c];
};

// Per-window item table: 0x80 item values, then the pointer that receives the chosen result.
struct GameMenuItems
{
    int values[0x80];
    int *result;
    int m_0204;
};

// Map table entry (0xe8 bytes): map number, four script-flag conditions and the map's names.
struct GameNamedEntry
{
    int map;                    // map number (9999 = any)
    int flags[4];               // script flags that must pass (see TestScriptFlag)
    char name[0x20];            // 0x14: map name used by scripts
    char file[0x20];            // 0x34
    char title[0x50];           // 0x54
    int value;                  // 0xa4
    char pad_00a8[0x40];
};

// Empty engine base class; its destructor is the shared empty function 0x1013eda0.
struct Object
{
    ~Object();
};

// One scene node body (0x18a0 bytes, destructor 0x10191480, see src/node_dtor.cpp). Game owns 32 of them.
struct GameNodeBody
{
    char data[0x18a0];
    ~GameNodeBody();
};

// The game object: global instance DAT_104b7200. Holds menu textures, HUD state, the script variable tables and
// 32 node bodies for menu models.
struct Game : Object
{
    int m_0000;
    int m_0004;
    int texMenu1;
    int texMenu4;
    int m_0010;
    int texMenu2;
    int texMenu5;
    int texBalloon5;
    int texBalloon6;
    int texFrame;
    int m_0028;
    int texMap01;
    int texMap02;
    int texUdChip;
    int texWaku01;
    int texWaku02;
    int texWaku03;
    int texWaku04;
    int texIcon01;
    int texWaku05;
    char pad_0050[0x2c];
    int texBalloon7;
    int texMenu15;
    int texMenu14;
    int m_0088;
    int m_008c[3];
    int m_0098[3];
    int m_00a4[3];              // 0xa4: id of the menu window in each slot
    int m_00b0[3];              // 0xb0: window x
    int m_00bc[3];              // 0xbc: window y
    int m_00c8[3];              // 0xc8: window width
    int m_00d4[3];              // 0xd4: window height
    char pad_00e0[0xc];
    int m_00ec[3];
    int m_00f8[3];
    int m_0104;                 // 0x104: set to 10 when a window opens
    int pad_0108[2];
    int m_0110[3];              // 0x110: 1 while the window slot is open
    int m_011c[3];
    int m_0128[3];
    int m_0134[3];
    int m_0140[3];              // 0x140: cursor position in the window (restored from m_0434 when remembered)
    int pad_014c[3];
    int m_0158[3];
    int m_0164[3];
    char pad_0170[0x18];
    int m_0188[3];
    int m_0194[3];
    int m_01a0[3];
    int m_01ac[3];
    int m_01b8[3];
    int m_01c4[3];
    int m_01d0[3];
    char m_01dc[0x258];
    char m_0434[0x258];
    char m_068c[4];
    int m_0690;
    char pad_0694[8];
    char m_069c[3][0x80][0x36]; // 0x69c: per-window item labels (0x80 items of 0x36 characters)
    GameMenuItems m_579c[3];    // 0x579c: per-window item values and result pointer
    int m_5db4[3][0x82];
    char pad_63cc[0xc30];
    int m_6ffc;
    char pad_7000[0x630];
    int m_7630;                 // 0x7630: menu cursor target x
    int m_7634;                 // 0x7634: menu cursor target y
    int m_7638;                 // 0x7638: menu cursor x (follows the target)
    int m_763c;                 // 0x763c: menu cursor y
    int m_7640;
    int m_7644;
    int m_7648;
    int m_764c;
    int m_7650;
    int m_7654[3];
    void (*m_7660)(int);
    int m_7664;
    float m_7668[3];            // 0x7668: per-window fade
    char pad_7674[0x11f4];
    GameNodeBody m_8868[0x20];  // 0x8868: menu model nodes, destroyed in ~Game
    char pad_39c68[0x5c1ac];
    GameNamedEntry m_95e14[0x100];
    int m_a4614;
    char pad_a4618[0xaf114];
    GameTrigger triggers[3400]; // 0x15372c: event triggers of the loaded map
    int triggerCount;           // 0x1f976c
    char pad_1f9770[0x61a84];
    GameTrigger mapEvents[2000];// 0x25b1f4: named map events
    int mapEventCount;          // 0x2bcc74
    char pad_2bcc78[0x130d04];
    char *text;                 // 0x3ed97c: script text being tokenised
    int pos;                    // 0x3ed980: read position in `text`
    int end;                    // 0x3ed984: text length
    char token[0xd00];          // 0x3ed988: the token read last
    int currentMap;             // 0x3ee688: number of the loaded map
    int pad_3ee68c[2];
    int m_3ee694[4000];

    ~Game();
    int FindSlot(int id);
    void M_10007f20();
    int LoadMenuTextures(int value);
    // Tests script flag `index` (4000 flags in m_3ee694, saved with the game). A negative index tests that the
    // flag is clear. Flag 0 is always set; out-of-range indexes read as clear (or as set for the negated test).
    // Inline: callers expand it (the out-of-line copy is at 0x100022b0).
    int TestScriptFlag(int index)
    {
        if (index < 0)
        {
            index = -index;
            if (index > 3999)
                return 1;
            return m_3ee694[index] == 0;
        }
        if (index > 3999)
            return 0;
        if (index == 0)
            return 1;
        return m_3ee694[index];
    }
    int ForEachActiveTrigger(void (*callback)(GameTrigger *trigger));
    int RaiseKskonEvents(int argument);
    int RaiseTalkEvents(int argument);
    int RaiseOpenEvents(int argument);
    int RegisterEvent(int id);  // 0x100020d0 (flag_list.cpp: FlagOwner::M_100020d0)
    int RaiseGameOverEvents();
    int FindMapEntry(int map, char *name, char *file, char *title, int unused1, int unused2, int unused3, int *value);
    int M_10002300(int index, int value);
    int ReadWord();
    int ReadWordInLine();
    int M_10007ee0();
    void M_100082e0();
    int M_10008390(int id, int value);
    int M_10008460(int id);
    int M_10008770(int id, int value);
    int M_100087b0(int id, int count);
    int M_100087f0(int x, int y, int w, int h, int frame, float alpha);
    int M_10008880(int x, int y, int w, int h, int frame, float alpha);
    int DrawLabelledNumber(int value, int x, int y, int frame, int color, float scale, float alpha);  // 0x10008a30
    int DrawNumber(int value, int x, int y, int frame, int color, float scale, float alpha);     // 0x10008c40
    int DrawNumberAlt(int value, int x, int y, int frame, int color, float scale, float alpha);  // 0x10008df0
    int MoveCursor();  // 0x10009050 (game_panels.cpp)
    int M_10008910(int x, int y, int lit, int rows, int a, int b, int color);
    int M_10008fa0(int x, int y, int color, int frame);
    int M_100091d0(const char *text, int x, int y);
    int M_10002060(const char *name);
    int M_100083e0(int id);
    int M_10008650(int id, int value);
    int M_100086b0(int id, int a, int b, int c, int d);
    int M_10008710(int id, int a, int b, int c);
    int OpenWindow(int id, int x, int y, int width, int height);  // 0x100084b0
    int M_10008330(int id, int *result);
    int M_1000b2c0(int id, const char *label, int value);
};

inline int Game::FindSlot(int id)
{
    int slot = 0;
    do
    {
        if (m_0110[slot] != 0 && m_00a4[slot] == id)
            break;
        slot++;
    } while (slot < 3);
    return slot;
}
