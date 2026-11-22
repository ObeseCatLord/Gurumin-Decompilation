# Session handoff

Rewrite this file at the end of every session (keep it short: state, not history). The next session starts by
reading it, then `AGENTS.md`.

## Progress (2026-11-22, `progress/jp/report.json`)
| measure | value |
|---|---|
| Fully linked | 0.00% of code (147 of 250 units complete) |
| Matched | 11.57% of code (199043 of 1720206 bytes), 789 of 1212 functions |
| Fuzzy match | 14.66% |

Under AGENTS.md rules 1-2 most matches are still MATCH-PENDING until their other types move into shared headers
(Phase 0 of `BACKLOG.md`).

## State
- **Vec4 merge done.** `include/vec4.h` holds the one Vec4 (plus `Box2`). 110 files include it, as does `world.h`
  (its `Float4` is now Vec4). `Scaled` became `operator*(const float &) const` (obj_symbols row
  `??DVec4@@QBE?AU0@ABM@Z 101acb00`) and `Normalize3` became `Normalized3`. No match was lost (`check_matches.py`
  exit 0 after every group). The only diff is fuzzy 14.68% → 14.66%, on functions that never matched.
- **One exception:** `src/npc_greet.cpp` keeps its own Vec4. With the shared header, Npc_Greet (0x100b8220) loses its
  match because register allocation tips (open.tsv). Retry it after the header settles: member order, or Copy as
  `operator=` (below).
- `scene_node.cpp`: the `bounds` union is now `Vec4 bounds[3]`, `PackHeader::vertices` is `PackVertex *`, and
  `LengthVector` is gone.
- Still duplicated small types: `Vec3` (7 files/headers), `ParamVector` in `char_params.h`. Its `operator=` at
  0x101acc20 suggests Vec4::Copy may really be `Vec4 &operator=(const Vec4 &)` (NOTES 2026-11-22).
- Raw offset casts left: `src/listbank.cpp` lines 760, 773, 796.

## Next steps
1. **Matrix4 merge**, the same way: about 65 files declare `struct Matrix4`/`Matrix` (`CharMatrix` in world.h and
   npc_greet.cpp is a view too). Survey the variants (member signatures vs the definers in `vec4.cpp`/`vec_math.cpp`),
   write `include/matrix4.h`, then groups of about 22 files per Sonnet subagent. Run in parallel on disjoint files;
   each agent uses unique temp script names. Each agent follows the per-file baseline → convert → rebuild →
   revert-on-loss loop. Gate each batch with `build_all.py` + `check_matches.py`.
2. `Vec3`, then `Node`, the D3D interface views, `CharRecord`, `NpcState`, `TextureObject`, `ModelBank`.
3. Then return to `QUEUE.md`.

## How the Vec4 merge was run (reuse for Matrix4)
- Brief per agent: list of files; for each, save the baseline MATCH lines (`build.bat <file>`), delete the local
  struct, `#include` the header where the struct stood, rename to the header's names, rebuild, compare. If something
  is lost, try at most two readable fixes; otherwise `git checkout` the file and log it to a shared TSV.
- Edit with binary-safe Python (keep CRLF/LF). `sed -i` flips line endings.
- `check_matches.py` reports a renamed symbol as LOST + new. If the address and the count are unchanged, it is a
  rename.

## Continuation prompt
"Continue the Gurumin Phase 0 type merge with Matrix4. Read `decomp/HANDOFF.md` and `AGENTS.md`. Survey the
Matrix4/Matrix declarations, write `include/matrix4.h`, then hand groups of about 22 files to parallel Sonnet
subagents (disjoint files, per-file baseline/convert/rebuild/revert-on-loss). After each batch run
`python tools/build_all.py --report ../progress/jp/report.json` and `python tools/check_matches.py` from `decomp/`.
Commit and push only when it exits 0. Rewrite HANDOFF.md at the end with the Fully linked / Matched / Fuzzy numbers."
