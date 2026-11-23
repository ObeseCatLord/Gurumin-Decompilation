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

## Progress (2026-11-23, `progress/jp/report.json`)
| measure | value |
|---|---|
| Fully linked | 0.00% of code (151 of 262 units complete) |
| Matched | 12.64% of code (217410 of 1720206 bytes), 802 of 1212 functions |
| Matched under the rules | 12.60% of code (216783 bytes), 801 of the 802 matched functions |
| Fuzzy match | 16.54% |

"Matched under the rules": a matched function counts only when its file declares no struct/class/union and no
`extern` global of its own (`tools/rule_check.py`; build_all prints a `RULES` line). Only npc_greet.cpp (1 matched
function) and node_world.cpp (0) still break rule 2.

## State
- This session (dcd0ede .. HEAD): **Enemy_StrafeAndLunge 0x10099040 matched** (2800 bytes) in the new rule-clean
  enemy_strafe.cpp (kinds 0x4c-0x4e, dispatched by char_dispatch.cpp); constants and enums in strafe_enemy.h;
  CharState::shotSound (0x18) and DAT_10467ba0 (char_tables.h) typed. Fuzzy 16.11% -> 16.54%.
  - **Enemy_StrafeAndShoot 0x1009a690** (kind 0x4c): 1 byte long, 426 diffs. Left: ebx/edi swapped all through
    (ours: index in edi, model offset in ebx) and the chase step's far/mid tails merged the other way round. Key
    finds that got it this far: `(float)atan2((double)x, (double)y)` (aligned frame + rounding, NOTES), the
    quarter turn is `3.1415f / 2`, and the attack step has its own `if (m_238c) AddMotionSpeed; return 0;`.
  - **Enemy_StrafeAndBurst 0x10099b30** (kind 0x4d): 47 bytes long, 559 diffs: the same register swap, and its
    idle block (`if (rand % 100 > 2) return 0;` before the motion-speed check) is merged into two copies instead
    of one. The defeated branch matches as an if/else with two full calls.
  - The swap appears only in the two functions with a common end check (`if (m_238c) AddMotionSpeed`); the
    matched twin returns from every path. open.tsv lists what was tried.
- **WorldRenderer::DrawPictures 0x10167440**, **Node::QueueDraw 0x10199e20** (twin 1019a6b0), **Node::BuildMesh
  0x101a82d0**: not changed this session (all three are scheduling/register walls; see open.tsv).
- **npc_greet.cpp** / **node_world.cpp** / **the region prologue wall** (0x101a8e60, RayCast 0x101a9670,
  CastSegment 0x101aaa20, CastSegment2 0x101aa080): unchanged (see open.tsv).
- Open match walls: Enemy_SeekShaga 0x100d2110, BoxSphere_Overlap 0x10192520, CylinderSphere_Overlap 0x10192c90,
  KeyBank::Sample 0x101af4c0, ProjectPlanarUVs 0x1019bdd0, plus the drafts above.
- Every function left in QUEUE.md is 750 bytes or more. Look-alike families left in sections 4-5:
  101713f0/10170580/10187520 (3.7 KB each), 100fda40/100fc4a0 (5 KB), 10167d90 (2.6 KB, WorldRenderer, its own
  shape). Kinds 0x47-0x51 (char_dispatch.cpp FUN_1009e2d0): 1009d490 (0x4f), 1009b120 (0x50), 1009c070 (0x51) are
  still in the queue and are probably more members of the enemy-behaviour family (check them against
  enemy_strafe.cpp and char_move.cpp's Enemy_*AndBlast first).

## Next steps
1. Kinds 0x4f-0x51 (1009d490, 1009b120, 1009c070): compare with enemy_strafe.cpp / char_move.cpp; a new member
   of an existing shape can match at the first build (10099040 did).
2. A new family in a new rule-clean file: 101713f0/10170580/10187520 (decode with `asm.py`, write, iterate).
3. The 1009a690/10099b30 register swap: look for a source shape of the common end check (or of the attack step's
   model pointer) that leaves the index in ebx; try the idle block of 10099b30 written another way.
4. DrawPictures / QueueDraw / BuildMesh, the region prologue and npc_greet only with a genuinely new idea.

## How the merges were run (reuse)
- Types: `python tools/type_move_driver.py apply <files>` then `python tools/prune_header_includes.py <new headers>`;
  views by hand (write the merged header, cut each copy, rename fields per file, build each file and compare its
  MATCH/DIFF lines with the HEAD version built as a scratch `src/zz_*.cpp`).
- Globals: `globals_driver.py plan|apply`, `use_header_globals.py`, `resolve_globals.py`. A global whose type is
  file-local cannot go into a shared header until the type does.
- Process files as bytes and keep each line's ending; never `sed -i` a CRLF file. Bash heredocs with `\n` inside
  Python strings get mangled on this machine: write scripts with the Write tool. Check `git diff --stat`.
- `check_matches.py` reports a renamed symbol as LOST + new: if the address and count are unchanged, update the
  `obj_symbols.tsv` row (this session: SceneNode/NodeBody -> Node, Bind's ModelRecord -> AnimModel).

## Continuation prompt
"Continue the Gurumin decompilation. Read `decomp/HANDOFF.md` (including its Standing instructions) and `AGENTS.md`.
Work through HANDOFF 'Next steps': first the enemy kinds 0x4f-0x51 (1009d490, 1009b120, 1009c070) against the
shapes in enemy_strafe.cpp and char_move.cpp, then a new section 4-5 look-alike family (101713f0/10170580/10187520)
in a new rule-clean file (decode the frame from `asm.py`, write, fix the stack layout with scopes and fresh locals as
NOTES describes), then the ebx/edi register swap of Enemy_StrafeAndShoot 0x1009a690 / Enemy_StrafeAndBurst
0x10099b30. Put every new type and global in a shared header so new matches count under the rules. After each batch
run `python tools/build_all.py --report ../progress/jp/report.json` and `python tools/check_matches.py` from
`decomp/`; commit and push only with no real loss (a rename shows as LOST + new at the same address: update
`obj_symbols.tsv`). Keep everything human-readable (named constants, liberal comments, informative names). Work
autonomously. Rewrite HANDOFF.md at the end with the Fully linked / Matched / Matched under the rules / Fuzzy
numbers, keeping its Standing instructions section and a continuation prompt for the next task, then end your turn."
