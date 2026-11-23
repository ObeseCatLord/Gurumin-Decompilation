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
| Fully linked | 0.00% of code (148 of 255 units complete) |
| Matched | 12.17% of code (209406 of 1720206 bytes), 797 of 1212 functions |
| Matched under the rules | 11.37% of code (195512 bytes), 770 of the 797 matched functions |
| Fuzzy match | 15.57% |

"Matched under the rules": a matched function counts only when its file declares no struct/class/union and no
`extern` global of its own (`tools/rule_check.py`; build_all prints a `RULES` line). `python tools/rule_check.py`
lists the files still breaking rule 2 (27 matched functions left), most matches first.

## State
- This session (37b7412, d9c2a00): Node::SegmentHitsMesh 0x101a8780 matched in the new node_mesh_hits.cpp
  (rule-clean). Node::m_0070 is now `triangleSpheres`; Node::IntersectTriangle 0x1019dca0 (ray-triangle test,
  writes the hit point into its third argument) is declared in node.h. Node::Collide 0x101a8e60 is drafted in the
  same file with the new signature `Collide(const char *tag, unsigned mask, Vec4 *from, Vec4 *to, Vec4 *hit)`
  (FUN_1001fff0 passes tag/mask through).
- **The region prologue wall** (open.tsv 0x101a8e60): Collide's body after the prologue is SegmentHitsMesh's
  matched code, but the prologue that picks the tag offset (node kind < 0x40 = visibility cell, "CDLnn" = collision
  region, kind 0xfd / "OvF" = offset 7) does not cross-jump like the original. The original shares the tails of the
  four branches (cell list DAT_11b002c0 vs DAT_11afffe8/DAT_11affdd8): `mov bl,[cell]; test bl,bl; mov ecx,6; je
  children; cmp al,0xfd; jne; mov ecx,7`. cl here keeps the cell-list branch as `cmp byte [..],0` and does not merge
  identical tails (checked with a toy file: shared char local, assignment after the test, ternary on the list,
  `&&`-chained gotos, an inline helper). RayCast 0x101a9670, CastSegment 0x101aaa20 and CastSegment2 0x101aa080 have
  the same prologue (with kind 0xff for name nodes and `(flags & mask) == mask`), so one idea that fixes it gives four
  functions (~9500 bytes).
- Rule merges this session: ScreenBatch was a view of TriangleBatch (0x10172ac0 = TriangleBatch::AppendVertices,
  `vertices` is `ScreenVertex *`); DAT_10202464 (int[4]), DAT_108029b8 (Vec4, player start), DAT_1042de28
  (char[0x80], items obtained) in game_vars.h; DAT_1020d860 in ground_height.h; names_od globals in name_paths.h.
  save_slot, screen_vertices and names_od are rule-clean now.
- Deferred / open (the 27 pending matches, from `rule_check.py`):
  - **Node family**: Node/NodeBody/SceneNode/WorldNode/GameNodeBody views (ctor 0x1019ebc0, dtor 0x10191480) are
    still separate; risky because Game and WorldRenderer embed them.
  - **WorldRenderer** (world_ctor, world_frame, scene_start, world_model_state; Bind takes ModelRecord = AnimModel).
  - **window_create.cpp**: RenderWindow/GameWindow views + DAT_10f1a860.
  - **DAT_10210fc0 map table** (0x100-byte entries read as MapEntry/DamageZone/CameraPoint/RoutePoint[16]): needs a
    union type (map_icons, map_route, route_follow).
  - **Switch hold slots** (switches.cpp, switch_effects.cpp, game_reset.cpp): DAT_1030bbf8/10255b08/10219fc0/
    1042e0a8/10219c40 are 0x80-slot tables, but game_reset memsets only 0x100 bytes (0x40 slots, a leftover of a
    smaller table): declare them [0x80] and write game_reset's memset size explicitly; DAT_1030bbf8 is
    `SwitchObject *` vs `int`; MapObject has views in switches/map_special/map_unload.
  - game_reset.cpp also: DAT_10453208 (int[0x4000] memset vs MessageText[0x7c] in message_open: sizes disagree),
    DAT_104b7200 (GameObject vs Game). DAT_104c03c0 (MapObject vs MapRegion, region_view/map_unload), npc_greet
    (Character/CharMatrix views), node_world (Keys/Channel).
- Open match walls: the region prologue (above), Enemy_SeekShaga 0x100d2110 (push placement), BoxSphere_Overlap
  0x10192520 and CylinderSphere_Overlap 0x10192c90 (y-sum operand order), KeyBank::Sample 0x101af4c0 (flag fold;
  KeyTrack twins 0x101afe00/0x101af960/0x101b0240 wait for it), ProjectPlanarUVs 0x1019bdd0 (x87 scheduling).
- Look-alike families left in sections 4-5: 101aa080/101aaa20/101a9670 (+101a8e60, region prologue),
  101713f0/10170580/10187520, 10167440/10167d90, 10199e20/1019a6b0, 1009a690/10099b30, 100fda40/100fc4a0.
  Playback::Update 0x1016a480 (1045 bytes, model_state.cpp neighbour) was looked at: many cross-jumped tails.

## Next steps
1. The region prologue (one fix = Collide, RayCast, CastSegment, CastSegment2): try a source shape where the
   four branches share code by construction, e.g. compute the cell entry into one variable in the branches and test
   it once after them, or test the tag offset in a separate statement; build a toy file in `src/` (delete it after)
   to compare shapes quickly, as this session did.
2. Another look-alike family: 101713f0/10170580/10187520 or 1009a690/10099b30 (decode the frame from `asm.py`).
3. Rule merges: the switch hold-slot globals (above), then the DAT_10210fc0 map-table union, WorldRenderer,
   window_create; then the Node family merge.
4. Section 4 functions next to rule-clean code; the open walls above if a new idea comes up.

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
Work through HANDOFF 'Next steps': first the region prologue shared by Node::Collide 0x101a8e60 and the
RayCast/CastSegment/CastSegment2 family (try new source shapes in a toy file), then another look-alike family from
'State', then the rule merges listed under 'Deferred / open' (switch hold-slot globals first) and `decomp/QUEUE.md`
section 4 functions next to already-matched rule-clean code (decode the frame from `asm.py`, write, fix scheduling
with variants). Put every new type and global in a shared header so new matches count under the rules. After each
batch run `python tools/build_all.py --report ../progress/jp/report.json` and `python tools/check_matches.py` from
`decomp/`; commit and push only with no real loss. Keep everything human-readable (named constants, liberal
comments, informative names). Work autonomously. Rewrite HANDOFF.md at the end with the Fully linked / Matched /
Matched under the rules / Fuzzy numbers, keeping its Standing instructions section and a continuation prompt for
the next task, then end your turn."
