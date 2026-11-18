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
