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
| Fuzzy match | 16.60% |

"Matched under the rules": a matched function counts only when its file declares no struct/class/union and no
`extern` global of its own (`tools/rule_check.py`; build_all prints a `RULES` line). Only npc_greet.cpp (1 matched
function) and node_world.cpp (0) still break rule 2.

## State
- This session (872a0cf .. HEAD): **Enemy_ChaseAndSlash 0x1009d490** (kind 0x4f) drafted in the new rule-clean
  enemy_slash.cpp (constants and enums in slash_enemy.h, CharState::stuckFrames added at 0x08). It is its own shape
  (guard / ready / slash with a hand-to-forearm trail effect / taunt / three side-step states), not a strafe twin.
  Frame size, zero register and the whole effect block match; it is 64 bytes short because the compiler
  cross-jumps the side-step states 6-8 differently (ours: 6+7 merged from RotateZ; the original: 8 into 7 from the
  heading fadd, 7 into 6 from the `< 200` test). open.tsv lists what was tried. NOTES gained two habits (CSE vs
  the zero register; store order inside cross-jumped tails is canonicalised).
- **1009b120 (kind 0x50, 3.9 KB)** is a strafe-family enemy with an extra hop/landing part (CharState 0x24/0x2c/0x34,
  LastPosition slots 0x18/0x28, ground probe FUN_101641b0) and an effect 0x57 attack on attackTimer 14 (sound
  0x153); **1009c070 (kind 0x51, 4.2 KB)** is a flying enemy (height clamped to +-5 a frame towards player z + 40 /
  + 200). Neither was started.
- **101713f0 / 10170580 / 10187520**: trail-effect renderers with a 0x1600-byte frame (`__chkstk`), effect
  definitions at DAT_1101dfdc (0x3d8 each), point lists at DAT_118e99f8 (0x4014 per entry); not started.
- Enemy_StrafeAndShoot 0x1009a690 / Enemy_StrafeAndBurst 0x10099b30 (ebx/edi swap), DrawPictures 0x10167440,
  Node::QueueDraw 0x10199e20, Node::BuildMesh 0x101a82d0, npc_greet.cpp, node_world.cpp and the region prologue
  wall: unchanged (see open.tsv).
- Open match walls: Enemy_SeekShaga 0x100d2110, BoxSphere_Overlap 0x10192520, CylinderSphere_Overlap 0x10192c90,
  KeyBank::Sample 0x101af4c0, ProjectPlanarUVs 0x1019bdd0, plus the drafts above.

## Next steps
1. 1009b120 (kind 0x50): start from enemy_strafe.cpp's Enemy_StrafeAndLunge (same chase/strafe steps) and add the
   hop part; new constants in strafe_enemy.h, new CharState names in char_state.h.
2. The trail renderer family 101713f0/10170580/10187520 in a new rule-clean file (decode the frame with `asm.py`;
   locals at 0x1600 need the big arrays declared in the original order).
3. 1009d490's side-step tails: try writing state 6 differently from 7/8 (only 7 and 8 are identical in the
   original) without changing behaviour; then the 1009a690/10099b30 register swap.
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
Work through HANDOFF 'Next steps': first kind 0x50 (1009b120) from the Enemy_StrafeAndLunge shape in
enemy_strafe.cpp plus its hop part, then the trail renderer family 101713f0/10170580/10187520 in a new rule-clean file
(decode the frame from `asm.py`, write, fix the stack layout with scopes and fresh locals as NOTES describes), then
the side-step tail merging of Enemy_ChaseAndSlash 0x1009d490 (enemy_slash.cpp) and the ebx/edi swap of
0x1009a690/0x10099b30. Put every new type and global in a shared header so new matches count under the rules. After
each batch run `python tools/build_all.py --report ../progress/jp/report.json` and `python tools/check_matches.py`
from `decomp/`; commit and push only with no real loss (a rename shows as LOST + new at the same address: update
`obj_symbols.tsv`). Keep everything human-readable (named constants, liberal comments, informative names). Work
autonomously. Rewrite HANDOFF.md at the end with the Fully linked / Matched / Matched under the rules / Fuzzy
numbers, keeping its Standing instructions section and a continuation prompt for the next task, then end your turn."
