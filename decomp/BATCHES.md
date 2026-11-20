# Work batches (2026-11-18)

Short batches taken from `QUEUE.md` section 3 (medium functions) and `open.tsv`, grouped by system so that each
batch shares structures and callees. Every batch ends with: comments, names in symbols.tsv/obj_symbols.tsv,
`build_all.py --report`, `sync_symbols.py`, `gen_queue.py`, commit and push. Functions that only match with
unreadable source go to `open.tsv`.

| # | Batch | Functions |
|---|---|---|
| 1 | Effect pool spawns | 1000dd90, 1000dba0, 1000df10, 1000e130 |
| 2 | Script tokenizers | 10035d20, 10035e80, 10035fa0, 10035b10 |
| 3 | Script parser pieces | 10036560, 100357a0, 10037560 |
| 4 | Game panels and HUD | 100084b0, 10008a30, 10008c40, 10008df0, 100092c0 |
| 5 | Message/HUD drawers | 1000c020, 1000c390, 1000cc30, 1000d6b0 |
| 6 | Data loaders | 1000beb0 TestDat_Load, 1000b060, 1000b2c0, 1000b4e0, 1000b750 |
| 7 | Character helpers | 10024d00, 1001c090, 1001c210, 1001b880 |
| 8 | Character movement | 100242c0, 100244b0, 10018100, 10011dd0 |
| 9 | Enemy helpers | 100543a0, 100541f0, 10034cc0, 10045c30, 10045ed0 |
| 10 | Item/list tables | 10054770, 10054d50, 10054f30, 10055110 |
| 11 | Table sweeps | 10055410, 10055690, 100558d0, 10055aa0, 10055ca0 |
| 12 | Scene node | 101ac010 Node::Load, 101ab9d0, 101ad350, 1019c3a0 |
| 13 | Scene node 2 | 10191500 Node::Init, 1019e9a0 Node::Release, 1019e610, 1019c690 |
| 14 | Anim model | 10162420, 10162610 PathMover::SetPosition, 101612b0, 10166f20 |
| 15 | Model bank / textures | 10169c60, 1016a2e0, 1016b050, 10175a90, 10175c90 |
| 16 | Renderer | 1015cae0, 10142b20, 10142dd0, 1013e260, 1013e790 |
| 17 | Sound and save | 10001cb0 Bgm_CheckHeader, 1003aef0 Bgm_Play, 10011240 Save_ReadThumbnail, 10012220 |
| 18 | Math and camera | 1002f320, 1002f050, 1002ee70, 1002ebf0, 10021c80 |
| 19 | Wall retries | the newest open.tsv entries with the NOTES patterns (MoveCursor, Char_ResetState, char_effects) |
| 20 | Naming sweep | `M_` methods and `DAT_` globals in the files touched by batches 1-19 |

### Status of batches 1-20 (2026-11-18)

Matched: Debris_Spawn, Debris_SpawnFromCharacter, Debris_SpawnFrom (1); ListBank::ReadStatement, ReadArgument,
ReadWord, ReadTerm (2); ListBank::GetIndexPart (3); Game::DrawLabelledNumber, DrawNumber, DrawNumberAlt (4);
Hud_DrawNameBanners, Drops_Update (5); Game::AddValueEntry, AddFlagEntry, AddKind5Entry, AddRangeEntry (6);
Char_PlaceOnGround, Npc_TalkFacingSimple, Npc_TalkFacing (7); Map_FindRegionAt (8); Enemy_Destroy (9);
PathMover::SetPosition, PathMover::Teleport (14); WorldRenderer::SetModelState (14).
Written, recorded in open.tsv: CharParams::operator= (needs a caller), Game::OpenWindow, Char_ProbeAhead,
Char_ProbeDirection, Npc_StandOnGround, Names_Expand, Screenshot_ReadBackBuffer, Region_WaveVertex.
Deferred (moved into batches 21-70): 1000dba0, 100092c0, 1000c020, 1000d6b0, 10037560, 100543a0, 10034cc0,
101ad350, 101612b0 (with the 10161781 fragment), 10142b20, 1016b050, 101670e0 (WorldRenderer constructor).
Batches 10 and 11 are map event raisers of the loop-pointer wall family (BACKLOG item 23).

## Batches 21-70 (found 2026-11-18)

Every remaining medium (M) and large (L) function of QUEUE.md, grouped by address range (one system each) so that a
batch shares structures and callees. Walls already recorded in open.tsv are left out; batch 10/11-style map event
raisers wait for the loop-pointer fix (BACKLOG item 23).

| # | System | Functions |
|---|---|---|
| 21 | game panels, HUD, menus | 10001170, 10001cb0 Bgm_CheckHeader, 1000baf0, 1000beb0 TestDat_Load, 1000c720 (L), 1000cfa1 (L) |
| 22 | game panels, HUD, menus | 1000e340 (L), 1000eed0 (L), 1000f5a0, 1000f7f0 (L), 1000fb80 |
| 23 | characters and collision | 100106e0 Save_Write (L), 10010d10 Save_ReadSlotInfo (L), 10011240 Save_ReadThumbnail, 10011520 (L), 10012220, 10012510 |
| 24 | characters and collision | 10012780, 10012990 (L), 10012d90 (L), 100131b0 (L), 100144c0 Ability_GetDescription (L), 10014d30 (L) |
| 25 | characters and collision | 100154c0, 10015960 (L), 10016700, 100169c0 (L), 10016ec0 (L), 10017d30 |
| 26 | characters and collision | 100184e0 (L), 10018870 (L), 10018fb0, 10019310 (L), 10019670 (L), 10019d80 (L) |
| 27 | characters and collision | 1001a0d0, 1001a990 (L), 1001b0a0 (L), 1001bb20 (L), 1001d2d0 (L), 1001d920 |
| 28 | characters and collision | 1001dc20 (L), 1001e4b0 (L), 1001ed80 (L), 1001f270, 1001f4d0 (L) |
| 29 | character movement and scripts | 10020090 (L), 10020560 (L), 10021380 (L), 10021c80, 10021e70 (L), 100236c0, 10023900 (L) |
| 30 | character movement and scripts | 10023d30 (L), 10024d00, 10025ce0 (L), 10026730 (L), 10026d80 (L), 10027630 (L), 100279b0 (L) |
| 31 | character movement and scripts | 10027da0 (L), 100286d0 (L), 10028fa0 (L), 10029730 (L), 10029cf0 (L), 1002a110, 1002a490 |
| 32 | character movement and scripts | 1002a740 (L), 1002cc70 (L), 1002ebf0, 1002ee70, 1002f050, 1002f320 |
| 33 | script parser, sound, saves | 100301a0 (L), 100308e0 (L), 10034210 (L), 10035050 (L), 100360f0 (L), 100366c0 (L) |
| 34 | script parser, sound, saves | 10036ca0 Script_RegisterBuiltin (L), 10037090 (L), 10037850 Script_InitLibrary, 10037b40 (L), 10038070 (L), 1003aef0 Bgm_Play |
| 35 | script parser, sound, saves | 1003b2c0 Save_Load (L), 1003b6e0 Costume_LoadPlayerModel (L), 1003bc60 (L), 1003c370, 1003fc40 (L) |
| 36 | items and effects | 10042140 (L), 100425f0 (L), 10043e20, 10043fc0 (L), 10044560 (L), 10044b10 (L), 100451c0 (L) |
| 37 | items and effects | 10045830 (L), 10045c30, 10045ed0, 100460d0 (L), 100469d0 (L), 10046f10 (L), 10047440 (L) |
| 38 | items and effects | 10047a10 (L), 10048050 (L), 10048b70, 10048e90 (L), 10049240 (L), 10049650 (L), 10049b00 (L) |
| 39 | items and effects | 1004cf20 (L), 1004d4b0 (L), 1004ed60 (L), 1004f250, 1004f4f0 (L), 1004f8b7 (L), 1004fbe0 (L) |
| 40 | map events and tables | 10050510 (L), 100509e0 (L), 10050df0 (L), 10051190 (L), 10051570 (L), 10051e20 (L) |
| 41 | map events and tables | 100524d0 (L), 10055ee0 (L), 10056260, 100564d0 (L), 10057c40, 10057ee0 (L) |
| 42 | map events and tables | 10058500, 1005c680 (L), 1005dbf0, 1005ecb0 (L), 1005f750 (L) |
| 43 | enemy and NPC behaviours | 100627d0 (L), 10065450 (L), 1007a370 (L), 1007ad20 (L), 1007b870 (L), 1007d0a0 (L), 10089210 (L) |
| 44 | enemy and NPC behaviours | 10098be0 (L), 10099040 (L), 10099b30 (L), 1009a690 (L), 1009d100 (L), 1009d490 (L), 1009ddd0 |
| 45 | enemy and NPC behaviours | 1009dfd0, 100a3900 (L), 100a6a50 (L), 100a7150 (L), 100a77f0 (L), 100a7db0 (L), 100a8370 (L) |
| 46 | enemy and NPC behaviours | 100a8ac0 (L), 100b0250 (L), 100b0880 (L), 100b0f00 (L), 100b1520, 100b1800 (L), 100b1ff0 (L) |
| 47 | enemy and NPC behaviours | 100b23b0 (L), 100b2900 (L), 100b3430 (L), 100b3770 (L), 100b4300 (L), 100b4930 (L), 100b4f00 (L) |
| 48 | enemy and NPC behaviours | 100b5550 (L), 100b58a0, 100b5c30 (L), 100b5f80, 100b6290 (L), 100b7360 (L), 100b7b10 (L) |
| 49 | enemy and NPC behaviours | 100b8220, 100b9150 (L), 100b9930, 100b9bb0 (L), 100ba040 |
| 50 | scene and game flow | 100d2110 (L), 100dfcf0 (L), 100e1c40 Enemy_IsBusy (L), 100e21a0, 100ed510 (L), 100ed9b0 Script_CompileStatement (L) |
| 51 | scene and game flow | 10115cd0 (L), 10137b10 Screenshot_Capture (L), 1013ad90 Script_RegisterGameBuiltins (L), 1013e260, 1013f070 (L) |
| 52 | renderer and textures | 10142790 (L), 10142dd0, 10143180 (L), 10143bc0, 10143e00, 10144320 (L), 101446f0 (L) |
| 53 | renderer and textures | 10144bf0, 101453e0 Font_DrawGlyph, 101455d0 Font_Init (L), 10146db0 (L), 10147a20 (L), 10148590, 10148b00 Render_CreateDevice (L) |
| 54 | renderer and textures | 10149100, 1014ade0 (L), 1014b480, 1014b750, 1014bc60 (L), 1014c080 (L), 1014c430 (L) |
| 55 | renderer and textures | 1014c960 (L), 1014ce40 (L), 1014d4e0 (L), 1014d850 (L), 1014ddf0 (L), 1014e5d0 (L), 1014e940 |
| 56 | renderer and textures | 1014ec30, 1014f1f0 (L), 1014f590 (L), 1014f900 (L), 1014fdd0 (L), 101501f0 (L), 10150bb0 (L) |
| 57 | renderer and textures | 10151150 (L), 1015c450, 1015cae0, 1015f4e0 (L), 1015fea0 SystemSav_WriteRecord (L) |
| 58 | models and effects | 101603c0 SystemSav_ReadRecord (L), 10160a90 Mmv_Open (L), 10161f80 (L), 101629f0, 10163150 (L), 101643c0 (L) |
| 59 | models and effects | 10164c10 (L), 10165240 (L), 10165e00, 10166100 (L), 10166520, 10166950 Character_Load (L) |
| 60 | models and effects | 10167440 (L), 10167d90 (L), 101687e0 (L), 101693f0, 10169c60, 1016a2e0 |
| 61 | models and effects | 1016a480 (L), 1016a980, 1016ac90, 1016b400, 1016b6c0 (L), 1016be80 (L) |
| 62 | models and effects | 1016fb80 (L), 10172350 (L), 10172ac0, 10172cb0 (L), 10175a90, 10175c90 |
| 63 | models and effects | 1017b210 (L), 1017b9b0 (L), 1017c160 (L), 1017c5f0, 1017c9c0 |
| 64 | effects, scene nodes, maths | 101886d0 (L), 101890f0, 10189340, 10190e30 (L), 10191500 Node::Init, 10191fd0, 10192200 |
| 65 | effects, scene nodes, maths | 10192520 (L), 10192a00, 10192c90 (L), 10193140 (L), 10193d10 (L), 10194a10, 10194c60 |
| 66 | effects, scene nodes, maths | 10199380, 101995a0, 101999a0, 10199e20 (L), 1019a6b0 (L), 1019b350, 1019b630 |
| 67 | effects, scene nodes, maths | 1019b890 (L), 1019bdd0 (L), 1019c3a0, 1019c690, 1019d8e0 (L), 1019dca0 (L), 1019e120 (L) |
| 68 | effects, scene nodes, maths | 1019e610, 1019e9a0 Node::Release, 1019eeb0 (L), 101a0cf0 (L), 101a24e0 (L), 101a29e0 (L), 101a4a60 It3_ScanChunks (L) |
| 69 | effects, scene nodes, maths | 101a53b0 (L), 101a82d0 (L), 101a8780 (L), 101a8e60 Node::Collide (L), 101a9670 (L), 101aa080 (L), 101aaa20 (L) |
| 70 | effects, scene nodes, maths | 101ab3e0 (L), 101ab9d0, 101abe30 Model_LoadIt3, 101ac010 Node::Load, 101af4c0 KeyBank::Sample (L), 101af960 (L), 101afe00 (L) |

### Progress on batches 21-70 (2026-11-18)

Matched: Loading_Tick (21), Char_SetHandTool, Char_SetHandToolPlain, Char_WearHelmet (23-25), Node::Draw,
DrawList_DrawPass (66), Node::Load (70), Node::SetupSkins (70), NodeExtra::AnimatedMatrix (60), Render_ResetSwapTarget
(54), Camera_Ease (26), Npc_TalkWhenNear (48). Written and recorded in open.tsv: Bgm_CheckHeader, TestDat_Load,
Model_ApplyCommands, Helmet_FaceSettings, Pickup_Collect, Node::LoadIt3, Node::FindBone, Circle_Intersect,
Enemy_CanReturnHome. Naming: 18 Game window/HUD methods and six TextureObject draw methods.

## Batches 71-120 (generated 2026-11-18)

Functions not in batches 1-70, in address order, then the open walls to retry with the patterns added to
NOTES.md this session (shared pushes from per-branch calls, low-byte shifts, negated NaN comparisons, scoped
copies, inline float helpers). (L) = over 1500 bytes.

| # | Area | Functions |
|---|---|---|
| 71 | new functions | 10002440 (L), 10004b10 (L), 100094e0 (L) |
| 72 | new functions | 1000a0b0 (L), 1000cf80, 100135b0 (L) |
| 73 | new functions | 10017c55, 1001c670 (L), 10022830 (L) |
| 74 | new functions | 10024f80 (L), 1002ae70 (L), 1002d4e0 (L) |
| 75 | new functions | 1002f520 (L), 100311b0 (L), 10032240 (L) |
| 76 | new functions | 10038c10 (L), 1003c570 (L), 1003e710 (L) |
| 77 | new functions | 100404e0 (L), 10042a30 (L), 10049ec0 (L) |
| 78 | new functions | 1004d960 (L), 1004f860, 10053020 (L) |
| 79 | new functions | 10056b30 (L), 10058790 (L), 1005ce80 (L) |
| 80 | new functions | 1005def0 (L), 1005fc00 (L), 100615f0 (L) |
| 81 | new functions | 10062e70 (L), 10064630 (L), 10065bf0 (L) |
| 82 | new functions | 100675d0 (L), 10068f70 (L), 1006be40 (L) |
| 83 | new functions | 1006e150 (L), 1006f490 (L), 10070200 (L) |
| 84 | new functions | 10071ee0 (L), 10075350 (L), 10076790 (L) |
| 85 | new functions | 10078060 (L), 10079470 (L), 1007c050 (L) |
| 86 | new functions | 1007dab0 (L), 1007f1e0 (L), 10083eb0 (L) |
| 87 | new functions | 10084e80 (L), 10086380 (L), 10089a10 (L) |
| 88 | new functions | 1008c680 (L), 1008fea0 (L), 10092af0 (L) |
| 89 | new functions | 10093cc0 (L), 1009b120 (L), 1009c070 (L) |
| 90 | new functions | 1009e380 (L), 100a4440 (L), 100a5960 (L) |
| 91 | new functions | 100a9240 (L), 100aa820 (L), 100ac440 (L) |
| 92 | new functions | 100ad0f0 (L), 100ade30 (L), 100af1b0 (L) |
| 93 | new functions | 100b6740 (L), 100b84a0 (L), 100ba2a0 (L) |
| 94 | new functions | 100bcce0 (L), 100bdad0 (L), 100c39c0 (L) |
| 95 | new functions | 100cb0c0 (L), 100cc2a0 (L), 100d24a0 (L) |
| 96 | new functions | 100e0320 (L), 100e25f0 (L), 100e7280 (L) |
| 97 | new functions | 100edea0 (L), 100f5a80 (L), 100f9670 (L) |
| 98 | new functions | 100fb7b0 (L), 100fc4a0 (L), 100fda40 (L) |
| 99 | new functions | 100fedd0 (L), 101011b0 (L), 10104640 (L) |
| 100 | new functions | 1010c650 (L), 10112140 (L), 10114950 (L) |
| 101 | new functions | 10116550 (L), 1011a060 (L), 1011d810 (L) |
| 102 | new functions | 10138280 (L), 1013b700 (L), 1013fbb0 (L) |
| 103 | new functions | 10145ba0 (L), 101493f0 (L), 10151ba0 (L) |
| 104 | new functions | 1015d520 (L), 1016c3e0 (L), 10170580 (L) |
| 105 | new functions | 101713f0 (L), 10173550 (L), 10174ce0 (L) |
| 106 | new functions | 10175f20 (L), 1017a010 (L), 1017cc90 (L) |
| 107 | new functions | 10184440 (L), 10187520 (L), 10189530 (L) |
| 108 | new functions | 10194f40 (L), 1019c8d0 (L), 1019f340 (L) |
| 109 | new functions | 101a1640 (L), 101a3000 (L), 101a58a0 (L) |
| 110 | walls to retry with the 2026-11-18 patterns | 100014b0, 100016c0, 10001980, 10001cb0, 10004760, 100084b0, 10008910, 10009050, 100091d0, 1000b9f0 |
| 111 | walls to retry with the 2026-11-18 patterns | 1000beb0, 1000e870, 1000ed20, 1000f360, 10011dd0, 10012130, 10012220, 100143f0, 10014d30, 10015710 |
| 112 | walls to retry with the 2026-11-18 patterns | 100157b0, 10015910, 1001b670, 1001b7b0, 100211a0, 100242c0, 100244b0, 10024d00, 1002a340, 1002ee70 |
| 113 | walls to retry with the 2026-11-18 patterns | 10034c80, 10034cc0, 100357a0, 10035a30, 10037470, 1003ab40, 10043cc0, 10045ed0, 10054550, 10054970 |
| 114 | walls to retry with the 2026-11-18 patterns | 10054a60, 10054b70, 10054c60, 10054d50, 10054f30, 10055110, 100bd960, 100e2080, 100e2110, 100e2440 |
| 115 | walls to retry with the 2026-11-18 patterns | 100e24d0, 100e2560, 100fb6d0, 1013e4f0, 1013e790, 1013e9d0, 10143870, 10144040, 101441b0, 10144ee0 |
| 116 | walls to retry with the 2026-11-18 patterns | 10145380, 1014b390, 1015cac0, 1015fb20, 1015fbf0, 10160960, 10160f00, 10161120, 10161200, 101614c0 |
| 117 | walls to retry with the 2026-11-18 patterns | 101615d0, 10163e90, 10163f90, 101640b0, 101649c0, 10164af0, 10165020, 10166870, 10167390, 10169720 |
| 118 | walls to retry with the 2026-11-18 patterns | 10169e10, 1016a020, 1016a090, 1016a100, 1016b010, 1016b1e0, 1016c300, 10175c90, 101883b0, 10188580 |
| 119 | walls to retry with the 2026-11-18 patterns | 10191fd0, 10194a10, 10194c60, 101992f0, 10199380, 10199880, 10199be0, 1019b280, 1019c560, 101abe30 |
| 120 | walls to retry with the 2026-11-18 patterns | 101acc60, 101ad240, 101ad2c0, 101ad5c0, 101ad920, 101ae950, 101aed50, 101aeee0 |

### Progress (2026-11-18, second half)

Matched: Node::SortPacks, Node::ResetVertexColours, InputState::SetButtons, Camera_TurnPush, ScreenBatch::Add,
EffectManager::DrawSpriteA/B, Route_Follow, Char_CanSee, Sound_UpdateListener, ListBank::InitLibrary,
Script_RegisterGameBuiltins, Item_DrawCaption. Written and recorded in open.tsv: Node::UpdateWorld, Box_Overlap,
Game::RaiseFireEvents/RaiseLeverEvents/RaiseDoorEvents, Texture_CreateFromPixels, Char_UpdateShoves,
Party_BlocksMove, Npc_ApproachPlayer, Playback::Play, MotionList::FrameAt, Char_WalkRoute, Joystick_IsButtonDown,
Save_ReadThumbnail.

## Batches 121+ (2026-11-18, rolling)

Each finished batch adds two new ones, taken from the medium-function section of QUEUE.md.

| # | Area | Functions | Result |
|---|---|---|---|
| 121 | textures, user folder, balloons | 10169c60 TextureObject::Create, 1013e260 UserFolder_FindNt, 100543a0 Balloon_Blast | Balloon_Blast matched; the other two open (block placement) |
| 122 | medium functions | 100558d0 ListBank::FindCase, 1000c020 Text_Substitute, 101612b0 Rle_Decode | Rle_Decode matched; FindCase open (jump threading); Text_Substitute deferred to batch 125 (eight loops) |
| 123 | medium functions | 1000dba0 Fade_Draw, 10017d30 RenderTarget_Probe, 10055aa0 | Fade_Draw matched; RenderTarget_Probe open (2 bytes); 10055aa0 (label search like FindCase) moved to batch 124 |
| 124 | medium functions | 10055690 Game::IsQueuedEventReady, 10055410, 10055ca0, 10055aa0 | IsQueuedEventReady open (trigger-loop pointer base; game.h gained queuedTriggers and queuedEvents); 10055410 has the same loop, left; 10055ca0/10055aa0 (keyword scanners) left |
| 125 | medium functions | 1000c020 Text_Substitute, 10054770, 1009ddd0 Kind4a_Update | Kind4a_Update matched; Text_Substitute open (15 bytes); 10054770 is Game::RaiseDeadEvents (same loop wall as the other Raise*Events), left |
| 126 | medium functions | 10175a90 Texture_CreateCopy, 1019e610, 100092c0 | Texture_CreateCopy matched; 1019e610 (vertex de-duplication, three nested loops) and 100092c0 (window scroll arrows over Game's per-window field arrays) left |
| 127 | medium functions | 100236c0 Overlay_DrawModel, 1000f5a0, 1001f270 | Overlay_DrawModel matched; 1000f5a0 (camera matrices from the map's camera node) and 1001f270 (nearest movie screen and its fade) left: many unnamed globals |
| 128 | medium functions | 1017c9c0, 1015c450 DShowStream::Open, 101629f0 | DShowStream::Open open (cross-jumped failure messages); its Close (was SurfaceSet::Release) moved into dshow_stream.cpp; 1017c9c0 (big object reset) and 101629f0 (model constructor) need their class layouts first |
| 129 | medium functions | 100ba040, 1019b630, 1003aef0 Bgm_Play | Bgm_Play matched; 100ba040 (talk prompt near NPCs, many globals) and 1019b630 (recursive bone blend) left |
| 130 | medium functions | 100b8220 Npc_Greet, 1002ebf0 DamageZone_Update, 100b9930 | Npc_Greet matched; DamageZone_Update open (index register); 100b9930 (enemy retreat behaviour) left |
| 131 | medium functions | 10058500 Switch_UpdateEffects, 10192a00, 10045c30 | Switch_UpdateEffects matched; 10192a00 (cylinder overlap) and 10045c30 (NPC watching the player) left |
| 132 | medium functions | 1004f250, 10057c40 Switch_Hold, 1000d6b0 | Switch_Hold open (slot register swap); 1004f250 (tutorial texture loader) and 1000d6b0 (key prompt drawing) left |
| 133 | medium functions | 1001d920, 100e21a0, 1002a490 Block_Update | Block_Update open (model address register); 1001d920 (camera target lock) and 100e21a0 (per-kind enemy hit reactions) left |
| 134 | medium functions | 1002f050, 10016700, 1016b400 Camera_TurnInput | Camera_TurnInput open (blend weight copy); 1002f050 (route point decoding) and 10016700 (ground height sampling) left |
| 135 | medium functions | 10001170 Obj::Update, 1014b750, 1016a980 Camera_Reset | Obj::Update and Camera_Reset matched; 1014b750 (scaled texture quad) left |
| 136 | medium functions | 10148590, 100b1520, 10142dd0 Keyboard_IsButtonDown | Keyboard_IsButtonDown open (shared return blocks); 10148590 (buffer pool eviction) and 100b1520 (enemy sight and alarm) left. Also matched from a scan: 10021380 Map_SetEntryFlags |
| 137 | medium functions | 10149100, 10037560, 10144bf0 | |
| 138 | medium functions | 1014e940 TextureObject::DrawFrameAlt, 1014ec30, 1017c5f0 | |
| 139 | medium functions | 10166520, 1005dbf0, 10165e00 | |
| 140 | medium functions | 10048b70, 10192200, 1000baf0 | |
| 141 | medium functions | 100b5f80, 1009dfd0, 101453e0 Font_DrawGlyph | |
| 142 | medium functions | 10161781, 1016b050, 101670e0 | |
| 143 | medium functions | 10055690 retry, 1013f070 neighbours: 10148590, 1014b750 | |
| 144 | medium functions | 1019b350, 10037560, 1014e940 | |
| 145 | medium functions | 1017c5f0, 10166520, 1005dbf0 | |
| 146 | medium functions | 10165e00, 1000baf0, 10192200 | |
| 147 | medium functions | 10048b70, 100b5f80, 1009dfd0 | |
| 148 | walls to retry | 10169c60, 1013e260, 10017d30, 10057c40 | |
| 149 | medium functions | 1014ec30, 10144bf0, 10149100 | |
| 150 | medium functions | 101670e0, 10169720 retry, 1015fea0 | |
| 151 | medium functions | 1003e710, 10161f80, 10163150 | |
| 152 | large, mechanical | 1003e710 Game_Reset, 10161f80 (model record reset, needs the record layout) | Game_Reset matched (5424 bytes), and with it CharParams::operator= (0x100357a0) |
| 153 | medium functions | 1003b2c0 Save_Load, 101455d0 Font_Init, 10160a90 MmvPlayer::Open | Save_Load and MmvPlayer::Open matched; Font_Init (font object layout) left |
| 154 | low-branch functions | 10146db0, 100106e0 Save_Write, 10010d10 Save_ReadSlotInfo | Save_Write open (unrolled loop registers); 10146db0 and Save_ReadSlotInfo (two save formats) left |
| 155 | low-branch functions | 101603c0 SystemSav_ReadRecord, 1015fea0 SystemSav_WriteRecord, 10166950 Character_Load | |
| 156 | save and system files | 10010d10 Save_ReadSlotInfo, 101603c0 SystemSav_ReadRecord, 101602d0 File_Checksum | |
| 157 | medium functions | 1004f4f0, 10023900, 10055ee0 | |
| 158 | large, mechanical | 10161f80 (model record reset), 1015d520 Text_ToFullWidth | Text_ToFullWidth matched (7664 bytes); 10161f80 AnimModel::ResetState matched (fields named by offset) |
| 159 | medium functions | 10146db0, 10023900, 1004f4f0 | |
| 160 | large, table-like | 101ab3e0, 1013b700 Game_Startup (10376 bytes), 10138280 Script_GameBuiltin (10107 bytes) | |
| 161 | large, table-like | 1011a060 (14244 bytes), 100fedd0 (6739 bytes), 10112140 Menu_Main | |
| 162 | model class | 101629f0 AnimModel::AnimModel (matched), 1017c9c0 (scene object reset, member arrays), 10166950 Character_Load | |
| 163 | large, table-like | 10116550 (staff roll, 15107 bytes), 10023900, 1004f4f0 | 10023900 Map_ResetOnEnter open (49 bytes: New Year block placement), 1004f4f0 TutorialBook_NextPage open (draft) |
| 164 | small helpers | cosf/floorf/sinf/sqrtf copies, 1013f880 Random, Vec4::Length, VertexPack/IndexList::Init, RenderWindow::WindowProc + GameWindow ctor, 10161970 | all 11 matched (File_Checksum was already matched); the game's window procedure is GameWindow::WindowProc |
| 166 | save and system files | 101603c0 SystemSav_ReadRecord, 1015fea0 SystemSav_WriteRecord, 10161770 (fragment check) | |
| 167 | medium functions | 101670e0, 1016b050, 101453e0 | 101670e0 WorldRenderer::WorldRenderer matched, and with its layout 1017c9c0 WorldRenderer::Bind; 1016b050 Camera_SelectPath open (shrink-wrapping); 101453e0 left |
| 168 | world renderer | 10166950 WorldRenderer::LoadCharacter, 10167390, 1017cc90 | LoadCharacter matched (1488 bytes); 10167390/10167400 are WorldRenderer::StartFade/TickFade (moved from model_params.cpp; StartFade open, 16 bytes); 1017cc90 (draw lists, very large) left |
| 170 | world renderer | 1017cc90 (draw-list builder), 10189000 WorldRenderer::Seek retry, 101692c0 | |
| 169 | medium functions | 101453e0 Font_DrawGlyph, 10055aa0, 10054770 | 10054770 Game::MarkKilled open (two-pointer loop, as RaiseGameOverEvents); 10143e00 Render_FillRect open (6 bytes); 10055aa0 (label search) and Font_DrawGlyph left |
| 171 | table-like | 100144c0 Ability_GetDescription (generated from the asm skeleton), 1019b630, 10055410 | Ability_GetDescription matched (2146 bytes; int parameter); the others left |
| 172 | string-heavy | 1001d2d0 (digit counter), 10034920 (BS_003 variants), 100b9bb0 | 10034920 Bs003_SetVariant matched; 1001d2d0 DigitCounter_Update open (register swap); 100b9bb0 left |
| 173 | string-heavy | 1001e4b0, 1017c160, 1004ed60 | 1017c160 ModelBank::CreateTextures matched (first build); others left |
| 174 | string-heavy | 100135b0 Map_ShowEquipment, 1001e4b0, 1001a990 | Map_ShowEquipment open (3600 bytes, cross-jump direction); 1001e4b0 (records text) and 1001a990 (texture names by kind) left |
| 175 | init-like | 1003b6e0 Costume_LoadPlayerModel, 100b0f00, 100b0250 | none (branchy character logic); left |
| 176 | save and system files | 1015fea0 SystemSav_WriteRecord, 101603c0 SystemSav_ReadRecord, 10015910 retry | WriteRecord matched (FILETIME times[4]); ReadRecord open (20 bytes, return block order); 10015910 still threads its tests |
| 177 | save files | 10010d10 Save_ReadSlotInfo, 1001b0a0, 1004f250 | Save_ReadSlotInfo open (stack layout, as Save_ReadThumbnail); the others left |
| 178 | node functions | 1019e610 Node::WeldVertices, 1004ed60, 10055410 | WeldVertices matched (first build); others left |
| 179 | node functions | 1019eeb0, 1019b630, 1019b350 | 1019eeb0 Node::SetupSkinning open (one null test); Node::M_1019ece0 renamed Find (returns Node*); others left |
| 180 | node functions | 1019b350 Node::CopyMesh, 1019b630, 1019b890 | CopyMesh matched (Node bounds as a Vec4[3] union); others left |
| 181 | node functions | 1019b630 Node::BlendFrames, 1019b890, 101a53b0 | BlendFrames matched (Node::poses[16] at 0x2d4, frames at 0x290); others left |
| 182 | node functions | 1019b890 Node::CopyAnimation, 101a53b0, 1019dca0 | CopyAnimation matched (reading `frame` per branch fixed the registers); others left |
| 183 | node functions | 101a53b0 Node::Clone, 1019dca0, 1019d8e0 | Clone matched (first build); others left |
| 184 | node and effect functions | 1019d8e0 Node::ComputeBounds, 10190e30 EffectManager::Draw, 1019dca0 | ComputeBounds and Draw open (register/scheduling residue); 1019dca0 (ray-triangle) left |
| 185 | engine functions | 1019e120 Node::BuildCollisionSpheres, 1016ac90 Camera_CollectPaths, 10192200 | Camera_CollectPaths matched (first build); BuildCollisionSpheres open (14 bytes, address-mode operand order); 10192200 (collision geometry) left |
| 186 | integer functions | 101455d0 Font_Init, 10023d30 Map_Unload, 10142790 | Font_Init (FontRenderer layout filled in) and Map_Unload open (register choice); 10142790 left |
| 187 | integer functions | 10142790, 1014f1f0 TextureObject::Upload, 10148590 | 10148590 TexturePool_Get open; the other two left (DirectInput, pixel loops) |
| 188 | wall retries | 1015cac0, 10169e10, 10034c80, 10166870, 1016b010 | four matched (float setter, method call, two store orders); 1016b010 left (eax/edx) |
| 189 | wall retries | 1000ed20, 1016a090, 10012130, 1016b1e0 | 1000ed20 (counter read into a local) and 1016b1e0 (two ternaries) matched; 1016a090 and 10012130 still open |
| 190 | wall retries | 1016b010, 100e2080 family, 10015710, 101992f0 | all still open: control-flow, store-order, result-variable and flag variants leave the zero/1.0f register choice unchanged |
| 191 | small/medium functions | 1000cf80, 1004f860, 10017c55, 10055aa0 | 10055aa0 ListBank::FindLabel matched and FindCase 0x100558d0 with it (`*text` tests, no jump threading); 1004f860 TutorialBook_PrevPage open; 1000cf80/10017c55 are mis-sized fragments |
| 192 | medium functions | 10055410, 10055ca0, 10161781 | 10055410 Game::RunQueuedEvents (trigger pointer taken after the id test) and 10055ca0 ListBank::SkipStatement (declaration order) matched; 10161781 is a fragment |
| 193 | medium functions | 101453e0 Font_DrawGlyph, 100092c0, 1000f5a0 | 101453e0 and 100092c0 Game::DrawWindowMarks open (near-matches); the Game::Draw* colour argument is a float (callers push floats), retyped without losing matches |
| 195 | medium functions | 1000f5a0, 1000d6b0, 1001d920 | 1001d920 Carry_StackOnObject and 1000d6b0 KeyPrompt_DrawAll matched (float comparison spellings) |
| 197 | medium functions | 1000f5a0, 10045c30, 1004f250 | |
| 194 | medium functions | 1001f270, 100ba040, 10055 neighbours | 1001f270 Map_SelectMovie open (registers; it is a C++ function, extern "C" drops its EH frame) |
| 196 | wall retries | 10055690, Raise*Events | IsQueuedEventReady, RaiseGameOver/Kskon/Talk/OpenEvents matched (element pointer after the first test); Fire/Lever/Door left (inline budget) |
| 198 | medium functions | 100ba040, 100b9930, 10192a00 | 10192a00 Cylinder_Overlap matched (block-scoped temporaries) and wall 10191fd0 Box_Overlap with it (separate early returns) |
| 199 | medium functions | 100ba040, 100b9930, 1004f250 | 100b9930 Enemy_DragToPit open (one float copy) |
| 201 | medium functions | 100ba040, 1004f250, 10045c30 | 100ba040 Npc_PlayGreeting open; pick.py now keeps line endings |
| 202 | medium functions | 1004f250, 10045c30, 10037560 | 10045c30 Npc_WaitFacingAway matched (if/else calls, block-scoped turn, no pointer local) |
| 203 | medium functions | 1004f250, 10037560, 10016700 | 1004f250 TutorialBook_Close open (registers) |
| 204 | medium functions | 10037560, 10016700, 1014b750 | 10037560 ListBank::AssignValue open (frame layout) |
| 205 | medium functions | 10016700, 1014b750, 100b1520 | 10016700 Ground_HeightAt open (float copy pattern shared with two other drafts) |
| 206 | ListBank | 10037090, 100360f0 (1014b750 skipped: quad-draw register wall family) | 10037090 FindVariable open (search-loop registers; ReportError found to be inline) |
| 208 | large functions | 100360f0, 100b9bb0, 10055ee0 | 100b9bb0 Enemy_PlayVoice open (jump threading); 100360f0 and 10055ee0 left (goto-heavy) |
| 209 | wall retries | walls with a ternary or a pointer local (if/else calls, no pointer local) | 1019c560 Node::ApplyMaterialColours matched (no pointer local); 1000f360 down to an esi/edi swap |
| 210 | wall retries + medium | 1003ab40, 101ad350, 1002f050 | walls unchanged (register allocation); 1002f050 Route_FollowGates open (registers) |
| 211 | near-miss walls | 10194c60, 1001b670, 101ad240, 1002ee70 (fewest differing bytes) | all residues confirmed: EH state numbering, slot order by first use, dot-product schedule, fadd schedule; 100e21a0 left (enemy AI zero-register family) |
| 212 | large functions | 10172350, 1001b0a0, 100366c0 | 10172350 EffectManager::Start studied (needs the 0x1dc-byte effect record laid out: matrix 0x104, options 0x17c..0x1c0, handle 0x1d8); not written yet |
| 213 | effect record layout | EffectRecordA fields for 10172350, then the function | effect_start.cpp lays out the record, definition and options; EffectManager::Start open (one store cluster) |
| 214 | ListBank | 10036ca0 (1016fa40/10172780 were already done) | 10036ca0 RegisterBuiltin open (shrink-wrapping); builtin tables laid out in listbank_vars.cpp |
| 215 | large functions | 1001b0a0, 100366c0, 10021e70 | 1001b0a0 Char_SelectKind open (register roles) |
| 216 | large functions | 100366c0, 10021e70, 1001a990 | 1001a990 Char_AttachWeapon matched (switch on the model count, goto past for the default) |
| 217 | large functions | 100366c0, 10021e70, 1004ed60 | studied: 100366c0 is ListBank::DeclareFunction (RegisterBuiltin's twin with parameter names at 0x1230c), 10021e70 the save menu's status digits; Enemy_PlayVoice switch forms retried (worse) |
| 218 | large functions + near-miss | 1004ed60, 100b0f00, 10143180, 100091d0 | studied only: 1004ed60 TutorialBook_Open (same register family as the other book functions), 100b0f00 (EH, enemy state), 10143180 input any-key poll; DrawDebugText statement orders tried |
| 219 | naming + study | the five unnamed matched functions; 100b0250/100b0880/100b0f00 | named all five; 100b0250, 100b0880 and 100b0f00 are three near-identical companion updates (EH, ~1600 bytes each), left for a dedicated batch |
| 220 | companion updates | 100b0250, 100b0880, 100b0f00 | Companion1_Update and Companion2_Update matched; Companion0_Update open (20 bytes, idle-branch temporaries) |
| 221 | large functions | 1015f4e0, 10147a20, 1005ecb0, 100b1520 | Npc_WatchForPlayer matched (one shared `DAT_10210b30[index] = 0` at the end instead of one per return kept ebp free of a cached zero); the other three studied only |
| 207 | wall study | the fld/fstp float-copy pattern (100ba040, 100b9930, 10016700) | found for 10016700 (sum of two casts spills the first); the other two are a different copy, still open |
| 200 | wall retries | float-compare walls, MarkKilled | MarkKilled and Vec4::InTriangle still open (register roles, return-block placement) |
| 165 | medium functions | 10055ee0, 10146db0, 10010d10 Save_ReadSlotInfo | |
| 222 | NPC behaviours | 100b5f80, 100b5c30, 100b5550, 100b3430 | Npc_AlertWhenSeen, Npc_TurnWhenSeen (C++, EH for the effect matrix) and Npc_RiseWhenSeen matched; Npc_GreetWhenNear open (index/record offset registers swapped) |
| 223 | NPC behaviours | 100b1ff0, 100b6290 | Npc_FaceAndTalk and Npc_RunToMarker matched (Vec4 0x101af080 normalises in place) |
| 224 | NPC behaviours | 100b23b0 | Npc_ChildFollow matched (per-return clears here, so the zero lives in ebp as in the original) |
| 225 | NPC behaviours | 100b4f00 | Npc_PopUp matched (C++, EH; a clamp written into a second float keeps the self-copy; `max(min(x, 20), 0)` macros) |
| 226 | NPC behaviours | 100b4300 | Npc_RaiseAlarm drafted (33 bytes, zero register around the alarm); the `||` form of a call test made cl 13.00 inline atan2f |
| 227 | NPC behaviours | 100b4930 | Npc_RideAlarm drafted (25 bytes: index/loop counter registers swapped); `if (a > c || b > d) X else Y` reproduces the original block order |
| 228 | enemy behaviours | 1009dfd0 | Enemy_FollowAndBlast matched (`rand() % 100 <= 2`, the record matrix is now the shared Matrix4 view) |
| 229 | enemy behaviours | 1009d100, 10098be0 | Enemy_ChaseAndBlast and Enemy_ShoveAndBlast matched (the latter with a direct `atan2` call where cl 13.00 would not expand `Atan2`) |
| 230 | character behaviours | 10048b70 | Char_SwingDoor matched (C++, EH; root node matrix = saved matrix * RotationY 0x101b1bb0) |
| 231 | character behaviours | 10045830 | Npc_SleepUntilWoken matched in its own unit (npc_sleep.cpp): a new CharRecord member in char_move.cpp flipped Npc_PopUp |
| 232 | player | 1005dbf0 | Player_TakeDamage drafted (player_damage.cpp); its hurt-voice helper will not inline under cl 13.00 |
| 233 | enemy behaviours | 10048e90 | Enemy_DropIn matched (switch with two cases carrying the same statement gives dec/je/dec/jne) |
| 234 | enemy behaviours | 10049240, 10049b00 | Enemy_DropInWhenNear matched; 10049b00 (route position interpolation, 0x40 bytes per point) studied only |
| 235 | enemy behaviours | 10046f10 | Enemy_AppearNearby matched in its own unit (enemy_blink.cpp) |
| 236 | enemy behaviours | 10044560 | Enemy_Pendulum matched (own unit; one function-scope `frames` shared by two states gives the original slots; fields read directly instead of an `angle` local) |
| 237 | behaviour dispatch | 100dfcf0 | Char_RunBehaviour matched (npc_dispatch.cpp): the behaviour table 1..0x31 now names every NPC behaviour by number |
| 238 | NPC behaviours | 100451c0, 10044b10 | Npc_WalkRoute drafted (route_carrier.cpp, frame 8 bytes larger); 10044b10 (another route walker) studied only |
| 239 | NPC behaviours | 10047a10, 100b4930 | Npc_RideBehindMarker drafted (4 bytes); sqrtf/atan2f order; Npc_RideAlarm down to its register swap |
| 240 | NPC behaviours | 100469d0, 10043fc0 | Npc_Held matched (npc_frozen.cpp: if/else around the two Start calls, ModelState follow flag as a bitfield); 10043fc0 studied only |
| 241 | NPC behaviours | 10047440 | Npc_PushCart drafted (npc_wheel.cpp; a spilled pointer local in the original) |
| 242 | camera | 10019d80, 100b1800 | Camera_Between matched (camera_follow.cpp); 100b1800 (group chat NPC, three unrolled scans) studied only |
| 243 | camera | 100184e0 | Camera_ItemStand matched (camera_follow.cpp) |
| 244 | map view | 1000f5a0 | Map_BuildTopView matched (map_view.cpp) |
| 245 | map view | 1000f7f0 | Map_DrawTopView matched (map_view.cpp, a void function) |
| 246 | attached meshes | 1000baf0 | MeshSlot::Draw matched (mesh_slots.cpp; double literals for the camera offsets) |
| 247 | enemies | 100e21a0 | Enemy_DefaultStep drafted (enemy_react.cpp; zero register) |
| 248 | walls retried | 1002ee70, 10034cc0, 10024d00 | Char_UpdateShoves and Party_BlocksMove matched with math.h atan2f; Enemy_CanReturnHome differs earlier |
| 249 | enemy moves | 10027630 | Enemy_ChasePlayer drafted (enemy_chase.cpp) |
| 250 | collision probes | 10050df0 | Char_ProbeSideLedge drafted (ledge_probe.cpp) |
