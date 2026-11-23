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
| Matched | 12.87% of code (221328 of 1720206 bytes), 803 of 1212 functions |
| Matched under the rules | 12.83% of code (220701 bytes), 802 of the 803 matched functions |
| Fuzzy match | 16.91% |

"Matched under the rules": a matched function counts only when its file declares no struct/class/union and no
`extern` global of its own (`tools/rule_check.py`; build_all prints a `RULES` line). Only npc_greet.cpp (1 matched
function) and node_world.cpp (0) still break rule 2.

## State
- **Enemy_HopAndStrafe 0x1009b120** (kind 0x50) matched in enemy_strafe.cpp: the Enemy_StrafeAndLunge shape plus a
  hop part (CharState hopRequests/restFrames/hopBack/hopFrame/hopHistory, LastPosition history2/history3, HopHistory
  enum and HOP_* constants in strafe_enemy.h, DAT_10254310 retyped as 0x80 bytes). Two lessons: a statement written
  in both branches of an if/else (SetTranslation) is cross-jumped, not hoisted; `if (byteArray[i] > 0)` gives `ja`.
- **Enemy_FlyAndStrafe 0x1009c070** (kind 0x51) drafted next to it: everything after the two height clamps matches
  (up to frame offsets). The clamps keep a temporary copy the compiler here never makes (open.tsv lists the 20
  spellings tried). AnimModel::M_101627d0 is now SetMoveStep(x, y, float z) (still matches).
- **101713f0 / 10170580 / 10187520** (trail renderers, 0x1600-byte frames, KeyBank locals built with the array
  constructor helper FUN_10004650, effect definitions at DAT_1101dfdc 0x3d8 each, point lists at DAT_118e99f8 0x4014
  each): not started; they need new types for both tables first.
- Enemy kinds of Enemy_UpdateBehaviour 0x100bcb30 (char_dispatch.cpp) are unstarted: 100a77f0 (1463 bytes, a
  special-attack wrapper around FUN_10076790 with Matrix4 temporaries and EH), 100a7db0 (1471, same family,
  "@firenull"), 100a7150 (1688), 100a6a50 (1792), 100a8370 (1866), and the large 10076790/100a5960/100a9240/
  100aa820/100ade30/100af1b0.
- Enemy_StrafeAndShoot 0x1009a690 / Enemy_StrafeAndBurst 0x10099b30 (ebx/edi swap), Enemy_ChaseAndSlash 0x1009d490
  (side-step tails), DrawPictures 0x10167440, Node::QueueDraw 0x10199e20, Node::BuildMesh 0x101a82d0, npc_greet.cpp,
  node_world.cpp and the region prologue wall: unchanged (see open.tsv).

## Next steps
1. The Enemy_UpdateBehaviour wrappers 100a77f0 and 100a7db0 (same family; decode the frame with `asm.py`, Matrix4
   temporaries via Matrix4()/~Matrix4() and RotationZ 0x101b1b40 as in companion_update.cpp), then 100a7150,
   100a6a50, 100a8370, in a new rule-clean file with their constants in a header.
2. The trail renderer family 101713f0/10170580/10187520: type the effect definition (0x3d8) and trail point list
   (0x4014) tables first, then the functions in a new rule-clean file.
3. Enemy_FlyAndStrafe's clamps with a genuinely new idea; 1009d490's side-step tails; the 1009a690/10099b30 swap.
4. Other section 4-5 functions next to matched rule-clean code (`decomp/QUEUE.md`).

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
Work through HANDOFF 'Next steps': first the Enemy_UpdateBehaviour wrappers 100a77f0 / 100a7db0 / 100a7150 /
100a6a50 / 100a8370 in a new rule-clean file, then the trail renderer family 101713f0/10170580/10187520 (type their
tables first), then the remaining near-misses with a genuinely new idea. Put every new type and global in a shared
header so new matches count under the rules. After each batch run `python tools/build_all.py --report
../progress/jp/report.json` and `python tools/check_matches.py` from `decomp/`; commit and push only with no real
loss (a rename shows as LOST + new at the same address: update `obj_symbols.tsv`). Keep everything human-readable
(named constants, liberal comments, informative names). Work autonomously. Rewrite HANDOFF.md after each batch with
the Fully linked / Matched / Matched under the rules / Fuzzy numbers, keeping its Standing instructions section and a
continuation prompt."
