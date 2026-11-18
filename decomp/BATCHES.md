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
