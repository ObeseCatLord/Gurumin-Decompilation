# EN work batches

One row per batch of the EN matching loop (`HANDOFF.md`). Every batch ends with `tools/build_all.py --report
../progress/en/report.json`, `tools/check_matches.py` (no LOST), `tools/gen_queue.py`, a commit and a push.

| # | Date | Functions | Result |
|---|---|---|---|
| 0 | 2026-11-24 | pipeline set-up; `Node::Find` 0x7532c0, `Map_RunFrameScript` 0x5dc5c0 | 2 matched (JP source ported unchanged) |
| 1-2 | 2026-11-24 | Node methods (27), Vec4/Matrix4/Box helpers (24), camera/IME (11), AnimModel channels (9), first containers | 82 matched (+80); `Vec4::RotateZ` open (17 bytes, xmm register swap); `port_jp.py` finds JP `M_<address>` sources; 8 map labels found wrong (named by the EN bytes) |
| 3-4 | 2026-11-24 | containers, texture manager, engine/render (IDirect3DDevice9), slot table, texcache, MMV, mesh cache, model bank, Game flags/windows, effect nodes | 151 matched (+69); `Render_CloseScene` recovered as an inline helper; `Game` layout equals JP at every offset used |
| 5 | 2026-11-24 | Node (26 more), AnimModel params (17), char state/effect slots/AI pick/visibility (14), Vec4/KeyBank/quaternion helpers (14), texture writes and draws (8), EffectModel ctor, ListBank (partial) | 248 matched (+97); gate's one LOST is the rename SetValue231c -> SetDropCollider (same address, still MATCH); several JP twins are EN widescreen-era rewrites (DrawTinted, SwapTarget preview, DrawFrame) |
