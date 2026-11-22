# Session handoff

Rewrite this file at the end of every session (keep it short: state, not history). The next session starts by
reading it, then `AGENTS.md`.

## State (2026-11-22)
- 789 of 1212 functions match byte for byte (11.57% of code). Under AGENTS.md rules 1-2 most of them are
  MATCH-PENDING until their types move into shared headers (Phase 0 of `BACKLOG.md`).
- Raw offset casts left: `src/listbank.cpp` lines 760, 773, 796.
- `include/vec4.h` exists (untracked, not yet included anywhere): the canonical Vec4, one name per address
  (`operator*(const float &) const` replaces `Scaled`; `Normalized3` replaces `Normalize3`).

## In progress: merge Vec4 into include/vec4.h
A first attempt converted 109 of 111 files with a script and lost 26 matches; it was reverted. Findings:
- `char_pit.cpp` and others declare `Matrix4` (which uses `Vec4`) above the file's last `#include`, so the include
  must go before the first struct that uses `Vec4`, not after the last `#include`.
- Calls `x.Scaled(expr)` with complex arguments were not all rewritten (`actor_scan.cpp` line 111).
- Lost after the attempt: model_bank.cpp (all ModelBank methods), the 0x100246b0-0x10024c40 collision probes,
  0x10011b70-0x10011d40, 0x1001a450, 0x1001ba40, 0x1001bec0, 0x10034e80, 0x100b8220, app_0001.cpp
  `Obj::ReportCursor`. Check whether these files lacked a Vec4 constructor (28 files did): adding the real
  constructor can change codegen there; if so the file needs a closer look, not a fake type.
- Hand work: `scene_node.cpp` (pointer-style `DistanceToSegment`, separate `LengthVector` type),
  `visibility.cpp` (`VisBox` instead of `Box2`).
- The `Scaled` definition in `vec4.cpp` becomes `operator*(const float &scale) const`; add
  `??DVec4@@QBE?AU0@ABM@Z<TAB>101acb00` to `obj_symbols.tsv`.

## Next steps
1. Finish the Vec4 merge file by file in small groups; after each group run `python tools/build_all.py --report
   ../progress/jp/report.json` and `python tools/check_matches.py` (it must exit 0: no match may be lost).
2. Then `Matrix4`, `Node`, the D3D interface views, `CharRecord`, `NpcState`, `TextureObject`, `ModelBank`.
3. Then return to `QUEUE.md`.

## Continuation prompt
"Continue the Gurumin Phase 0 type merge. Read `decomp/HANDOFF.md` and `AGENTS.md`. Plan the Vec4 merge in plan
mode (groups of about 15 files, the include placed before the first struct that uses Vec4, rename Scaled and
Normalize3 calls), then hand each group to a Sonnet subagent with the exact steps and the rule that no MATCH may be
lost. After each group run build_all.py and tools/check_matches.py; commit only when it exits 0, and rewrite
HANDOFF.md at the end."
