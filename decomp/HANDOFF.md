# Session handoff

Rewrite this file at the end of every session (keep it short: state, not history). The next session starts by
reading it, then `AGENTS.md`.

## Standing instructions (keep this section when rewriting the file)
- **Autonomous run (user, 2026-11-22):** work without waiting for approval; do not stop to ask questions a sensible
  default answers. When this session's task is done (or it can go no further), rewrite this file, commit and push,
  and end the turn; the orchestrating session starts the next one with the continuation prompt below.
- **Human-readable above all:** no magic numbers (named constants, enums or a comment saying what each value is),
  comments on every function (what it does, who calls it), on struct fields and on non-obvious logic, informative
  function and variable names (rename `FUN_`/`M_`/`DAT_` and `m_XXXX` when the evidence supports a name), early
  returns and plain loops. Comments and names never change compiled bytes; if a readable spelling loses a match,
  keep the readable one only when it is a comment/name change, otherwise record the function in `open.tsv`.
- Hard rules from `AGENTS.md` (typed members, one declaration per type, no fakematches, never commit a lost match).

## Progress (2026-11-22, `progress/jp/report.json`)
| measure | value |
|---|---|
| Fully linked | 0.00% of code (147 of 250 units complete) |
| Matched | 11.57% of code (199043 of 1720206 bytes), 789 of 1212 functions |
| Fuzzy match | 14.66% |

Under AGENTS.md rules 1-2 most matches are still MATCH-PENDING until their other types move into shared headers
(Phase 0 of `BACKLOG.md`).

## State
- **Vec4 merge done** (`include/vec4.h`, 110 files). **Matrix4 merge done** (`include/matrix4.h`, 77 files plus
  `world.h`, commits 6bf56b5 and 2f8989e). No match lost; four symbols were renamed (same address, count
  unchanged): `SetTranslation` now takes a `Vec4`, and `Draw@MeshSlot`, `StartProximityPass@Node` and
  `TagRegion@Node` mangle `Matrix4` instead of `Matrix`. `obj_symbols.tsv` holds only the new names.
- `Matrix4` has an anonymous union: `m[16]` plus D3DMATRIX-style `_11`..`_44`. The `Character` position union in
  `world.h` became plain `Matrix4 matrix` (constructors cannot sit in unions), so the position is
  `matrix._41/_42/_43`. `app_0010.cpp` and `pushing_platform.cpp` cast `&matrix._41` to `Vec3 *` for the camera
  list `DAT_1032d5b8`. Remove the cast when Vec3 is shared.
- **Exception:** `src/npc_greet.cpp` keeps its own Vec4 and CharMatrix (open.tsv 100b8220). Npc_Greet loses its match
  with `vec4.h` (register allocation tips). Retry it later, with member order or Copy as `operator=`.
- Still duplicated small types: `Vec3` (`char_script`, `damage_zone`, `head_look`, `pushing_platform`,
  `sound_effects`, `vec_math`, `world.h`), and `ParamVector` in `char_params.h`. `node_dtor.cpp` `Element`
  (float v[4], out-of-line destructor) may be Vec4 with an empty destructor (NOTES).
- Other views still per file: `CharMatrixView` (char_status.cpp), the `CharRecord` views that hold a matrix
  (char_script, char_shove, pushing_platform ...).
- Raw offset casts left: `src/listbank.cpp` lines 760, 773, 796.

## Next steps
1. **Vec3 merge**: write `include/vec3.h` (POD, x/y/z; `Vec3_Length`/`Vec3_Dot`/`Vec3_LengthSquared` are cdecl free
   functions in vec_math.cpp). Move world.h's Vec3 there, convert the 6 files, then drop the `(Vec3 *)` casts if
   a typed path exists (a `Vec3`-typed view of the translation row is not allowed in a union with Matrix4's
   constructor; keep the cast with its comment otherwise).
2. Then `Node`, the D3D interface views, `CharRecord`, `NpcState`, `TextureObject`, `ModelBank`.
3. Then return to `QUEUE.md`.

## How the merges were run (reuse)
- One Python driver (keep it outside the repo, e.g. in the session scratchpad) per type. For each file it: removes the
  local struct block (plus the comment lines directly above it), writes `#include "<type>.h"` where the struct stood,
  renames view names (`Matrix`, `CharMatrix`, ...) to the canonical one outside `//` comments, rebuilds with
  `cmd /c .\build.bat <file>`, and compares the matched addresses with the committed `progress/jp/report.json`
  (map mangled names to addresses through `obj_symbols.tsv`; `_FUN_xxxxxxxx` carries its own). On a loss or a
  compile error it runs `git checkout -- <file>` and logs the reason. Three copies ran in parallel on disjoint file
  lists; 67 of 69 files converted unattended, the other two needed a by-hand rename. This costs far fewer tokens than
  Sonnet subagents; use subagents only for files that the script reverts.
- Before converting, add `obj_symbols.tsv` rows for every C++ symbol whose mangled name contains the old view name
  (old rows stay until the end, then go).
- Edit with binary-safe Python (keep CRLF/LF). `sed -i` flips line endings.
- `check_matches.py` reports a renamed symbol as LOST + new. If the address and the count are unchanged, it is a
  rename; commit, then `check_matches.py` exits 0 against the new HEAD.
- A header that a shared header (`world.h`) includes must be adopted by that header's includers in the same step,
  or their local copies collide.

## Continuation prompt
"Continue the Gurumin Phase 0 type merge with Vec3. Read `decomp/HANDOFF.md` (including its Standing instructions)
and `AGENTS.md`. Survey the Vec3 declarations, write `include/vec3.h` (world.h includes it), convert the files with a
per-file baseline/convert/rebuild/revert-on-loss driver as described under 'How the merges were run', and use Sonnet
subagents only for files the driver reverts. Then continue with `Node`, the D3D interface views and `CharRecord` as
far as the session allows. After each batch run `python tools/build_all.py --report ../progress/jp/report.json` and
`python tools/check_matches.py` from `decomp/`; commit and push only with no real loss (renames at the same address
are fine). Keep everything human-readable (named constants instead of magic numbers, liberal comments, informative
names). Rewrite HANDOFF.md at the end with the Fully linked / Matched / Fuzzy numbers, keeping its Standing
instructions section and a continuation prompt for the next task."
