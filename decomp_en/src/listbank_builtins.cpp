// EN script parser setup: registers the game's builtins. JP twin: decomp/src/listbank.cpp (Script_RegisterGameBuiltins).

#include <string.h>
#include "listbank.h"
#include "script_builtins.h"

// Sets up the game's script parser: the library builtins, the keyword "ch" for the script flags (ch[n]), the
// message callback and the game builtins; the names declared by then count as global ones. Called once at
// startup, before any script is loaded.
extern "C" void ScriptParser_RegisterGameBuiltins(void)
{
    g_scriptParser.InitLibrary();
    g_scriptParser.m_36734[0] = g_scriptFlags;
    g_scriptParser.m_367b4[0] = 0;
    strcpy(g_scriptParser.m_36334[0], g_scriptFlagKeyword);
    g_scriptParser.failed = 0;
    g_scriptParser.callback = ScriptParser_Message;
    g_scriptParser.RegisterBuiltin("int Message(int,string,string,string,string)", GAME_BUILTIN_MESSAGE, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int ShopStart()", GAME_BUILTIN_SHOPSTART, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int ShopItem(int)", GAME_BUILTIN_SHOPITEM, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int ShopEnd()", GAME_BUILTIN_SHOPEND, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int ShopEnd2()", GAME_BUILTIN_SHOPEND2, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int SelStart(int)", GAME_BUILTIN_SELSTART, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int SelText(string,int)", GAME_BUILTIN_SELTEXT, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int SelEnd()", GAME_BUILTIN_SELEND, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int Wait(int)", GAME_BUILTIN_WAIT, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int WaitD(int)", GAME_BUILTIN_WAITD, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int WaitC(int)", GAME_BUILTIN_WAITC, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int LoadMap(int)", GAME_BUILTIN_LOADMAP, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int CharPos(int,int,int,int)", GAME_BUILTIN_CHARPOS, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int MapEnable(int)", GAME_BUILTIN_MAPENABLE, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int Lock()", GAME_BUILTIN_LOCK, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int Unlock()", GAME_BUILTIN_UNLOCK, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int Walk(int,int,int)", GAME_BUILTIN_WALK, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int Black(int,int,int)", GAME_BUILTIN_BLACK, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int Look(int,int)", GAME_BUILTIN_LOOK, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int BGM(int)", GAME_BUILTIN_BGM, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int SE(int)", GAME_BUILTIN_SE, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int Event(int)", GAME_BUILTIN_EVENT, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int Visible(int,int)", GAME_BUILTIN_VISIBLE, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int MapEvent(int)", GAME_BUILTIN_MAPEVENT, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int WMessage(string,string)", GAME_BUILTIN_WMESSAGE, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int Scop(int)", GAME_BUILTIN_SCOP, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int PlayMovie(string,int)", GAME_BUILTIN_PLAYMOVIE, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int CrossFade()", GAME_BUILTIN_CROSSFADE, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int Kaifuku()", GAME_BUILTIN_KAIFUKU, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int RCharEffect(int,int)", GAME_BUILTIN_RCHAREFFECT, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int Blur(int,int,int)", GAME_BUILTIN_BLUR, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int MMV(string)", GAME_BUILTIN_MMV, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int Soubi(int)", GAME_BUILTIN_SOUBI, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int StatusUp(int)", GAME_BUILTIN_STATUSUP, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int Camera(int,int,int,int)", GAME_BUILTIN_CAMERA, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int SetRoll(int,int)", GAME_BUILTIN_SETROLL, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int GetItem(int)", GAME_BUILTIN_GETITEM, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int Walk2(int,int,int,int)", GAME_BUILTIN_WALK2, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int Mes(string)", GAME_BUILTIN_MES, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int Menu(int)", GAME_BUILTIN_MENU, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int SetMapColor(int,float,float,float)", GAME_BUILTIN_SETMAPCOLOR, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int PostScr(int)", GAME_BUILTIN_POSTSCR, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int LockCommand(int)", GAME_BUILTIN_LOCKCOMMAND, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int GetMP()", GAME_BUILTIN_GETMP, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int SaveMap()", GAME_BUILTIN_SAVEMAP, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int RestoreMap()", GAME_BUILTIN_RESTOREMAP, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int LVUP(int)", GAME_BUILTIN_LVUP, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int SetSavePos()", GAME_BUILTIN_SETSAVEPOS, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int ResetFade()", GAME_BUILTIN_RESETFADE, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int GetGP()", GAME_BUILTIN_GETGP, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int SetGP(int)", GAME_BUILTIN_SETGP, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int KaguTest(int,int)", GAME_BUILTIN_KAGUTEST, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int KaguTest2(int,int,string)", GAME_BUILTIN_KAGUTEST2, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int White(int,int,int)", GAME_BUILTIN_WHITE, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int YukaKesi(int,int)", GAME_BUILTIN_YUKAKESI, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int REffect(int,int,int,int)", GAME_BUILTIN_REFFECT, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int SendEvent(int)", GAME_BUILTIN_SENDEVENT, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int LiftEnable(int)", GAME_BUILTIN_LIFTENABLE, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int Lift2C(int)", GAME_BUILTIN_LIFT2C, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int Door2C(int)", GAME_BUILTIN_DOOR2C, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int TekiStop(int)", GAME_BUILTIN_TEKISTOP, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int GetCharPos(int,int&,int&,int&)", GAME_BUILTIN_GETCHARPOS, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int LiftEnable2(int,int)", GAME_BUILTIN_LIFTENABLE2, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int Getbt15(int)", GAME_BUILTIN_GETBT15, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int Scop2(int,int,string)", GAME_BUILTIN_SCOP2, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int SaveCamera()", GAME_BUILTIN_SAVECAMERA, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int RestoreCamera()", GAME_BUILTIN_RESTORECAMERA, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int Miss()", GAME_BUILTIN_MISS, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int Reset()", GAME_BUILTIN_RESET, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int SetSavePos2(int,int,int,int)", GAME_BUILTIN_SETSAVEPOS2, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int GAMEOVER(int)", GAME_BUILTIN_GAMEOVER, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int Recover(int,int,float,float,float)", GAME_BUILTIN_RECOVER, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int Special(int)", GAME_BUILTIN_SPECIAL, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int Wait2(int)", GAME_BUILTIN_WAIT2, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int Takara2C(int)", GAME_BUILTIN_TAKARA2C, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int Demo(string)", GAME_BUILTIN_DEMO, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int LoadMapB(int)", GAME_BUILTIN_LOADMAPB, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("string StageName(int)", GAME_BUILTIN_STAGENAME, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("string StageMMV(int)", GAME_BUILTIN_STAGEMMV, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int ReloadChar()", GAME_BUILTIN_RELOADCHAR, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int PlayMMV3(string,int)", GAME_BUILTIN_PLAYMMV3, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int PlayAVI(string,int)", GAME_BUILTIN_PLAYAVI, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int Fill(int,float)", GAME_BUILTIN_FILL, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int Custom()", GAME_BUILTIN_CUSTOM, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("string Custom2(int)", GAME_BUILTIN_CUSTOM2, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int Custom3(int)", GAME_BUILTIN_CUSTOM3, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int Ksk(int)", GAME_BUILTIN_KSK, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int SetWaku(int)", GAME_BUILTIN_SETWAKU, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int Kabe(string)", GAME_BUILTIN_KABE, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int Mouse(int)", GAME_BUILTIN_MOUSE, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int PlayMovie2(string,int)", GAME_BUILTIN_PLAYMOVIE2, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("string Type(int)", GAME_BUILTIN_TYPE, RunGameBuiltin);
    g_scriptParser.RegisterBuiltin("int GetCf(int)", GAME_BUILTIN_GETCF, RunGameBuiltin);
    g_scriptParser.globalBuiltinCount = g_scriptParser.builtinCount;
    g_gameBuiltinsRegistered = 1;
}
