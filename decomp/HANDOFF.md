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
| Fully linked | 0.00% of code (149 of 256 units complete) |
| Matched | 12.24% of code (210547 of 1720206 bytes), 798 of 1212 functions |
| Matched under the rules | 12.20% of code (209920 bytes), 797 of the 798 matched functions |
| Fuzzy match | 15.64% |

"Matched under the rules": a matched function counts only when its file declares no struct/class/union and no
`extern` global of its own (`tools/rule_check.py`; build_all prints a `RULES` line). `python tools/rule_check.py`
lists the files still breaking rule 2: only npc_greet.cpp (1 matched function) and node_world.cpp (0) are left.

## State
- This session (e829cf3 .. HEAD):
  - Node::IntersectTriangle 0x1019dca0 matched (new node_triangle.cpp): Moller-Trumbore plus uv interpolation via
    two plane intersections. The checks are nested `if (!(u < 0) && !(u > det)) { ... }` (scopes give the stack
    slot sharing) and each interpolated uv got a fresh local (NOTES).
  - DAT_11affdd8 ("CDL region table") was a fake: it is DAT_11afffe8 (visible cells, camera_vars.h) indexed by
    `(name[3] - '0') * 10 + (name[4] - '0')`, the '0's folded into the displacement.
  - Rule merges, 771 -> 797 under the rules: switch hold slots (switch_slots.h, game_reset clears
    SWITCH_HOLD_SLOTS_CLEARED explicitly); MapObject views were Game (RaiseLeverEvents/RaiseDoorEvents/
    RegisterEvent) and MapRegion (`buffer0004`); GameObject -> Game::M_100082e0; message texts extern in
    message_text.h (MESSAGE_TEXT_CLEARED_BYTES); map record table = `union MapRecord` (map_records.h); Node family:
    SceneNode/NodeBody/GameNodeBody/WorldNode are Node (Node::Node 0x1019ebc0, ~Node 0x10191480; Node::box[2] replaces
    boundsMin/boundsMax because the ctor builds a two-element array with a loop); WorldRenderer's four views in
    world_renderer.h (ModelRecord = AnimModel, WorldCallback returns int); RenderWindow/GameWindow in render_window.h.
- **npc_greet.cpp** (open.tsv 0x100b8220, rules): matches only with its own Character view. Reading the character
  table through AnimModel changes register allocation; the cause is that AnimModel has union members (a test struct
  with the same offsets matches until any union or nested POD struct member is added). Either remove the four
  anonymous unions from AnimModel (check every AnimModel user still matches) or find another spelling.
- **node_world.cpp** (0 matched): Keys/Channel views.
- **The region prologue wall** (open.tsv 0x101a8e60, Collide; RayCast 0x101a9670, CastSegment 0x101aaa20 and
  CastSegment2 0x101aa080 share it). Original: both kind arms load the cell into bl (the register that caches
  DAT_11b04514) and share the tail; both CDL arms compare memory; the switched-off jump shares the tag-fail restore.
  Tried this session (all no better): local cell variable (gets dl), local initialised from DAT_11b04514, join-test
  after both arms (cl tail-duplicates it back), inline helper, bool cast, ternary, a switchedOff flag, a regionOn
  char tested in the main condition.
- Open match walls: Enemy_SeekShaga 0x100d2110 (push placement), BoxSphere_Overlap 0x10192520 and
  CylinderSphere_Overlap 0x10192c90 (y-sum operand order), KeyBank::Sample 0x101af4c0 (flag fold; KeyTrack twins wait
  for it), ProjectPlanarUVs 0x1019bdd0 (x87 scheduling).
- Look-alike families left in sections 4-5: 101aa080/101aaa20/101a9670 (+101a8e60), 101713f0/10170580/10187520,
  10167440/10167d90, 10199e20/1019a6b0, 1009a690/10099b30, 100fda40/100fc4a0, 101a24e0/101a29e0 (calls 50 each).
  101a82d0 (Node mesh rebuild, 0x28000-byte stack tables, 8x-unrolled count loops) was looked at.

## Next steps
1. Section 4 functions in node/vec code now that every node type is one Node: 101ab3e0 (calls IntersectTriangle),
   101a24e0/101a29e0 (pair), 10199e20/1019a6b0 (pair), 101a82d0; decode the frame from `asm.py`, write in a new
   rule-clean file, fix the stack layout with scopes (NOTES: nested ifs, fresh locals).
2. npc_greet: try AnimModel without its anonymous unions (each union -> its most used field, the other reads named
   by a comment or accessor) and rebuild everything; keep only if no match is lost.
3. The region prologue (four functions) only with a genuinely new idea (e.g. the whole own-triangle test in a
   nested `if` block with the children loop after it, or the prologue as an inline Node method returning the tag
   offset or -1).
4. node_world.cpp Keys/Channel views; then the open walls above.

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
Work through HANDOFF 'Next steps': first section 4 functions in the node/vec code (101ab3e0, the 101a24e0/101a29e0 and
10199e20/1019a6b0 pairs, 101a82d0) in new rule-clean files (decode the frame from `asm.py`, write, fix the stack
layout with scopes and fresh locals as NOTES describes), then npc_greet's AnimModel-union trap, then the region
prologue only with a new idea, then node_world.cpp's views and the open walls. Put every new type and global in a
shared header so new matches count under the rules. After each batch run
`python tools/build_all.py --report ../progress/jp/report.json` and `python tools/check_matches.py` from `decomp/`;
commit and push only with no real loss. Keep everything human-readable (named constants, liberal comments,
informative names). Work autonomously. Rewrite HANDOFF.md at the end with the Fully linked / Matched / Matched under
the rules / Fuzzy numbers, keeping its Standing instructions section and a continuation prompt for the next task,
then end your turn."
