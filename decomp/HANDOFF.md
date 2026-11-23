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
| Fully linked | 0.00% of code (147 of 252 units complete) |
| Matched | 12.07% of code (207655 of 1720206 bytes), 796 of 1212 functions |
| Matched under the rules | 11.16% of code (191918 bytes), 764 of the 796 matched functions |
| Fuzzy match | 15.44% |

"Matched under the rules": a matched function counts only when its file declares no struct/class/union and no
`extern` global of its own (`tools/rule_check.py`; build_all prints a `RULES` line). `python tools/rule_check.py`
lists the files still breaking rule 2 (32 matched functions left), most matches first.

## State
- Last session (cf657b4): AnimModel::CloneModel 0x1017b210 and AnimModel::LoadFile 0x1017b9b0 (was M_1017b9b0)
  matched in the new anim_clone.cpp, rule-clean (the 1017b210/1017b9b0 family is done). AnimModel fields named on
  the way (m_0013, m_0088, spineNode, leftToeNode, rightToeNode, colliderReach (float), m_2320, bipNode);
  DAT_10ff19e8 is `Matrix4 *[0x100]` (each model table entry's world matrix); Node::M_1019f340 takes a float reach.
- Enemy_SeekShaga 0x100d2110 drafted in shaga_seek.cpp (+ include/shaga.h: ShagaCandidate, SHAGA_SLOT_COUNT),
  rule-clean, body identical: ebx/edi are pushed before the FUN_100aa820 call (original: after the step test) and
  the pair branch's `count++` cross-jumps into the single branch. Early-return/nested-if/local placement/candidate
  spellings tried (open.tsv). A permuter run or a new idea about push sinking is the next lever.
- Deferred / open (the 32 pending matches, from `rule_check.py`):
  - **Node family**: Node/NodeBody/SceneNode/WorldNode/GameNodeBody views (ctor 0x1019ebc0, dtor 0x10191480) are
    still separate; risky because Game and WorldRenderer embed them.
  - **WorldRenderer** (world_ctor, world_frame, scene_start; Bind takes ModelRecord = AnimModel).
  - **window_create.cpp**: RenderWindow/GameWindow views + DAT_10f1a860.
  - **DAT_10210fc0 map table** (0x100-byte entries read as MapEntry/DamageZone/CameraPoint/RoutePoint[16]): needs a
    union type (map_icons, map_route, route_follow).
  - DAT_108029b8 (Vec4 read as raw ints in save_write/save_slot), DAT_104c03c0 (MapObject vs MapRegion),
    switches.cpp / switch_effects.cpp (MapObject + DAT_1030bbf8/10255b08/10219fc0/1042e0a8), game_reset.cpp
    (13 globals), names_od, npc_greet (skipped: Character/CharMatrix views), screen_vertices ScreenBatch.
  - GameFile 0x28 vs MovieFile 0x24; DAT_10874848/DAT_104b7200 both Game; ParamVector may be Vec4.
- Open match walls: Enemy_SeekShaga 0x100d2110 (above), BoxSphere_Overlap 0x10192520 and CylinderSphere_Overlap
  0x10192c90 (y-sum operand order), KeyBank::Sample 0x101af4c0 (flag fold; KeyTrack twins 0x101afe00/0x101af960/
  0x101b0240 wait for it), ProjectPlanarUVs 0x1019bdd0 (x87 scheduling). Details in open.tsv and NOTES.
- Section 4 candidates already looked at and left: 0x10164c10, 0x1001bb20, 0x10049b00, 0x10026730, 0x100360f0,
  0x1004f8b7, the 0x101a24e0/0x101a8780/0x101aa080 families. 0x10199e20 (Node::QueueDraw) has an EH frame and
  string tests; 0x10167440/0x10167d90 belong to an unnamed 0x14xxxx-byte object (no type yet).
- Look-alike families left in sections 4-5 (same instruction sequence, >85%): 101aa080/101aaa20/101a9670,
  101713f0/10170580/10187520, 10167440/10167d90, 10199e20/1019a6b0, 101a8780/101a8e60, 1009a690/10099b30,
  100fda40/100fc4a0. One match in a family usually gives the others (1017b210 gave 1017b9b0 at first build).

## Next steps
1. Another look-alike family: 101a8780/101a8e60 (Node::Collide, 1751 bytes; Node is a shared type) or
   10199e20/1019a6b0 (Node::QueueDraw; needs the EH frame), decoding the frame from `asm.py`.
2. Section 4 functions next to matched rule-clean code (anim_clone.cpp / texture_mgr.cpp / scene_node.cpp
   neighbours in QUEUE.md); fix scheduling with variants.
3. The remaining 32 pending matches: the DAT_10210fc0 map-table union (3 files), WorldRenderer (3 files),
   window_create, switches + switch_effects globals; then the Node family merge.
4. The open walls above (Enemy_SeekShaga push placement first), if a new idea comes up.

## How the merges were run (reuse)
- Types: `python tools/type_move_driver.py apply <files>` then `python tools/prune_header_includes.py <new headers>`;
  views by hand (write the merged header, cut each copy, rename fields per file, build each file).
- Globals: `globals_driver.py plan|apply`, `use_header_globals.py`, `resolve_globals.py` (above). A global whose type
  is file-local cannot go into a shared header until the type does.
- Process files as bytes and keep each line's ending; never `sed -i` a CRLF file (Git's sed also drops CRs of LF/CRLF
  headers). Bash heredocs with `\n`/`\r` inside Python strings get mangled on this machine: write scripts with the
  Write/Edit tools. Check `git diff --stat` before committing.
- `check_matches.py` reports a renamed symbol as LOST + new: if the address and count are unchanged, update the
  `obj_symbols.tsv` row. Compare fuzzy per address against HEAD as well.

## Continuation prompt
"Continue the Gurumin decompilation. Read `decomp/HANDOFF.md` (including its Standing instructions) and `AGENTS.md`.
Work through HANDOFF 'Next steps': one look-alike family from the list in 'State' (101a8780/101a8e60 first), then
`decomp/QUEUE.md` section 4 functions next to already-matched rule-clean code (decode the frame from `asm.py`, write,
fix scheduling with variants); put every new type and global in a shared header so new matches count under the
rules, and merge a file's remaining struct views (HANDOFF 'Deferred / open') when you touch it. After each batch run
`python tools/build_all.py --report ../progress/jp/report.json` and `python tools/check_matches.py` from `decomp/`;
commit and push only with no real loss. Keep everything human-readable (named constants, liberal comments,
informative names). Work autonomously. Rewrite HANDOFF.md at the end with the Fully linked / Matched / Matched under
the rules / Fuzzy numbers, keeping its Standing instructions section and a continuation prompt for the next task,
then end your turn."
