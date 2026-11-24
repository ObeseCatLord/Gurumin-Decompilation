// Game menu textures, reset and destructor (EN build). Ported from the matched JP source decomp/src/menu_textures.cpp.
#include <stdlib.h>
#include <string.h>

#include "engine.h"
#include "game.h"
#include "texture_mgr.h"

// Folder argument of Path_Data for the data images.
const int PATH_FOLDER_DATA = 0;
// No alpha image / no reload.
const char *const NO_ALPHA = 0;
const int NO_RELOAD = 0;
// Texture slot value of an unused texture field.
const int TEXTURE_NONE = -1;
// Bytes of scratch memory the loader allocates and frees again (leftover of the original).
const size_t MENU_SCRATCH_BYTES = 0x60000;

#define DATA(name) Path_Data(name, PATH_FOLDER_DATA)

// Loads every menu, balloon, map, frame and icon texture (BMP pairs with an alpha file) into the texture slots stored in
// the Game fields. Called once when the game starts.
int Game::LoadMenuTextures(FontRenderer *renderer)
{
    m_0690 = 0;
    ResetMenus();
    textRenderer = renderer;
    texMenu1 = Texture_LoadBmp(DATA("menu1.bmp"), DATA("menu1a.bmp"), NO_RELOAD);
    texMenu4 = Texture_LoadBmp(DATA("menu4.bmp"), NO_ALPHA, NO_RELOAD);
    texMenu2 = Texture_LoadBmp(DATA("menu2.bmp"), NO_ALPHA, NO_RELOAD);
    texMenu5 = Texture_LoadBmp(DATA("menu5.bmp"), NO_ALPHA, NO_RELOAD);
    m_0010 = TEXTURE_NONE;
    texBalloon5 = Texture_LoadBmp(DATA("fukida5p.bmp"), DATA("fukida5a.bmp"), NO_RELOAD);
    texBalloon6 = Texture_LoadBmp(DATA("fukida6p.bmp"), DATA("fukida6a.bmp"), NO_RELOAD);
    texFrame = Texture_LoadBmp(DATA("futi.bmp"), NO_ALPHA, NO_RELOAD);
    texMap01 = Texture_LoadBmp(DATA("map01.bmp"), NO_ALPHA, NO_RELOAD);
    texMap02 = Texture_LoadBmp(DATA("map02.bmp"), NO_ALPHA, NO_RELOAD);
    texUdChip = Texture_LoadBmp(DATA("UD_chip.bmp"), NO_ALPHA, NO_RELOAD);
    texWaku01 = Texture_LoadBmp(DATA("waku01.bmp"), DATA("WAKU01A.bmp"), NO_RELOAD);
    texWaku02 = Texture_LoadBmp(DATA("waku02.bmp"), DATA("WAKU02A.bmp"), NO_RELOAD);
    texWaku03 = Texture_LoadBmp(DATA("waku03.bmp"), DATA("WAKU03A.bmp"), NO_RELOAD);
    texWaku04 = Texture_LoadBmp(DATA("waku04.bmp"), NO_ALPHA, NO_RELOAD);
    texIcon01 = Texture_LoadBmp(DATA("ico01.bmp"), DATA("ico01a.bmp"), NO_RELOAD);
    texWaku05 = Texture_LoadBmp(DATA("waku05.bmp"), NO_ALPHA, NO_RELOAD);
    texBalloon7 = Texture_LoadBmp(DATA("fukida7p.bmp"), DATA("fukida7a.bmp"), NO_RELOAD);
    texMenu14 = Texture_LoadBmp(DATA("menu14.bmp"), NO_ALPHA, NO_RELOAD);
    texMenu15 = Texture_LoadBmp(DATA("menu15.bmp"), NO_ALPHA, NO_RELOAD);
    free(operator new(MENU_SCRATCH_BYTES));
    menuTexturesLoaded = 1;
    return 1;
}

// Resets the menu state: the three window slots, their item tables, the cursor and counters, and the cursor memory.
// Called when the menu textures load.
void Game::ResetMenus()
{
    m_0000 = 0;
    m_0004 = 0;
    m_7650 = 0;
    cursorX = 0;
    cursorY = 0;
    cursorTargetX = 0;
    cursorTargetY = 0;
    m_7648 = 0;
    m_764c = 0;
    m_7640 = 0;
    m_7644 = 0;
    activeWindow = 0;
    for (int i = 0; i < MENU_SLOT_COUNT; i++)
    {
        windowVisible[i] = 0;
        windowOpen[i] = 0;
        windowId[i] = 0;
        for (int entry = 0; entry < MENU_ENTRY_COUNT; entry++)
            entryKind[i][entry] = 0;
        windowFade[i] = 0;
    }
    for (int i = 0; i < MENU_CURSOR_CLEAR_INTS; i++)
        cursorMemory[i] = 0;
}

// Destructor: destroys the menu model nodes (compiler-generated eh vector destructor iterator), then the Object base.
// Nothing else needs releasing here; textures are freed through the texture manager.
Game::~Game()
{
}
