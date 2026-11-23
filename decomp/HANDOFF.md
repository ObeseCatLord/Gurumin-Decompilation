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
| Fully linked | 0.00% of code (151 of 260 units complete) |
| Matched | 12.48% of code (214610 of 1720206 bytes), 801 of 1212 functions |
| Matched under the rules | 12.44% of code (213983 bytes), 800 of the 801 matched functions |
| Fuzzy match | 16.11% |

"Matched under the rules": a matched function counts only when its file declares no struct/class/union and no
`extern` global of its own (`tools/rule_check.py`; build_all prints a `RULES` line). Only npc_greet.cpp (1 matched
function) and node_world.cpp (0) still break rule 2.

## State
- This session (5cb2834 .. HEAD): no new match; fuzzy 15.96% -> 16.11%.
  - **WorldRenderer::DrawPictures 0x10167440** drafted in new world_pictures.cpp (rule-clean, open.tsv): 21 bytes
    left, all in the in-world PICTURE_SCREEN_EXACT branch (the original starts the bottom-edge sum before pushing
    the two constant texture coordinates). The 0x20 script pictures are typed in world_renderer.h (pictureAlpha,
    pictureUVs, pictureAngle, pictureGrey, picturePositions, pictureSizes, pictureInWorld, pictureBlend,
    pictureKind, screenTexture/Width/Height; PICTURE_COUNT, PictureKind, PictureBlend). render_state.h gained
    RENDER_DEPTH_WRITE (9), RENDER_DEPTH_TEST (12), BLEND_ADD (1), BLEND_DARKEN (10).
  - **Node::BuildMesh 0x101a82d0**: the first vertex of each triangle now matches (index kept in `vertex`, no
    Vertex pointer). Left: `[index*0x74 + packs]` operand order (5 places) and the register of the third vertex
    index load. The add's spelling does not steer it (packs[i], i + packs, char* offsets in either order, an inline
    PackAt(packs, i), index types all compile identically); open.tsv lists what was tried.
  - **Node::QueueDraw 0x10199e20**: not changed. Diagnosis: ours keeps 0 in ebp (`cmp [mem], ebp`), the original
    has no zero register because ebp holds `view` and ebx `addColour` (in the queued branch the original calls
    `operator+` with `mov ecx, ebx`); the callee-saved pushes follow from that. Twin 1019a6b0 (with the CDL fade
    counter m_1899) waits for it.
  - 10167d90 is not DrawPictures' twin (EH frame, copies a matrix into the world, different loop).
- **npc_greet.cpp** / **node_world.cpp** / **the region prologue wall** (0x101a8e60, RayCast 0x101a9670,
  CastSegment 0x101aaa20, CastSegment2 0x101aa080): unchanged (see open.tsv).
- Open match walls: Enemy_SeekShaga 0x100d2110, BoxSphere_Overlap 0x10192520, CylinderSphere_Overlap 0x10192c90,
  KeyBank::Sample 0x101af4c0, ProjectPlanarUVs 0x1019bdd0, plus the drafts above.
- Every function left in QUEUE.md is 750 bytes or more. Look-alike families left in sections 4-5:
  101713f0/10170580/10187520 (3.7 KB each), 1009a690/10099b30 (2.7-2.9 KB), 100fda40/100fc4a0 (5 KB),
  10167d90 (2.6 KB, WorldRenderer, its own shape).

## Next steps
1. A new family in a new rule-clean file: 1009a690/10099b30 or 101713f0/10170580/10187520 (one source shape per
   family: match one, then the others are mostly field offsets). Decode with `asm.py`, write, iterate with sbs.
2. DrawPictures: one more idea for the in-world PICTURE_SCREEN_EXACT branch (e.g. branch order, or the v1 bug
   written another way); it is otherwise byte-identical.
3. QueueDraw: look for a source shape that gives `addColour` its own register in the queued branch (a separate
   variable per branch, or the colour defaults written inside the branches) before trying permutations.
4. BuildMesh, the region prologue and npc_greet only with a genuinely new idea.

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
Work through HANDOFF 'Next steps': first a new section 4-5 look-alike family (1009a690/10099b30 or
101713f0/10170580/10187520) in a new rule-clean file (decode the frame from `asm.py`, write, fix the stack layout
with scopes and fresh locals as NOTES describes), then the last 21 bytes of WorldRenderer::DrawPictures 0x10167440
(world_pictures.cpp), then Node::QueueDraw 0x10199e20's register allocation (addColour/view registers) and its twin
1019a6b0. Put every new type and global in a shared header so new matches count under the rules. After each batch
run `python tools/build_all.py --report ../progress/jp/report.json` and `python tools/check_matches.py` from
`decomp/`; commit and push only with no real loss (a rename shows as LOST + new at the same address: update
`obj_symbols.tsv`). Keep everything human-readable (named constants, liberal comments, informative names). Work
autonomously. Rewrite HANDOFF.md at the end with the Fully linked / Matched / Matched under the rules / Fuzzy
numbers, keeping its Standing instructions section and a continuation prompt for the next task, then end your turn."
