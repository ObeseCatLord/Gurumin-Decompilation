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
| Fuzzy match | 15.96% |

"Matched under the rules": a matched function counts only when its file declares no struct/class/union and no
`extern` global of its own (`tools/rule_check.py`; build_all prints a `RULES` line). `python tools/rule_check.py`
lists the files still breaking rule 2: only npc_greet.cpp (1 matched function) and node_world.cpp (0) are left.

## State
- This session (b7ce97a .. HEAD):
  - Matched: Node::RenderShadowTexture 0x101a24e0 and Node::DrawShadow 0x101a29e0 (new shadow_texture.cpp),
    Node::SegmentHitsBox 0x101ab3e0 (new node_box_hit.cpp). Float constants are `#define`s (a `static const float`
    is read from memory); the up vector is written before the copied caster position (NOTES).
  - Type work: ShadowScene and ModelData were Node (worldMatrix, bounds.w; model_bank_types.h deleted); Node::Draw
    and MeshData::Draw take typed shadow arguments (projector matrix, shadow centre floats, texture, light
    direction, model, alpha); render_state.h names FUN_1013f070's commands (blend, texture address, clear, flush,
    cull); fog: DAT_10f668f8 is a Vec4 fog colour, DAT_10f66974 float density, Node::fogColour/fogNear/fogFar/
    fogDensity and queueCallback (0x74); Vertex::m_0038 = sourceMaterial; PackHeaders/IndexLists_Alloc are the
    thiscall Node::AllocPackHeaders/AllocIndexLists (obj_symbols rows renamed).
  - Drafts (rule-clean, open.tsv): **Node::QueueDraw 0x10199e20** (node_queue.cpp; frame 0xb8 and block structure
    right; cl caches 0 in ebp where the original keeps `view` in ebp, `addColour` in ebx and pushes the callee-saved
    registers late; twin 1019a6b0 waits for it). **Node::BuildMesh 0x101a82d0** (node_mesh_build.cpp; all but the
    operand order of the VertexPack addresses, `[index*0x74 + packs]` in the original, 5 places).
- **npc_greet.cpp** (open.tsv 0x100b8220): AnimModel's own unions are not the only ones: AnimModel embeds Matrix4,
  which has a union too, so removing AnimModel's anonymous unions alone cannot help. Not tried further.
- **node_world.cpp**: its Keys view exists because MotionList::FrameAt (containers.cpp, matched) takes an int time
  while UpdateWorld passes the float DAT_11b044f8 bit-for-bit; one global used as two types. UpdateWorld is itself
  open (EH state numbering). Leave it unless the two-type global is understood.
- **The region prologue wall** (open.tsv 0x101a8e60; RayCast 0x101a9670, CastSegment 0x101aaa20, CastSegment2
  0x101aa080) unchanged this session.
- Open match walls: Enemy_SeekShaga 0x100d2110, BoxSphere_Overlap 0x10192520, CylinderSphere_Overlap 0x10192c90,
  KeyBank::Sample 0x101af4c0, ProjectPlanarUVs 0x1019bdd0, plus the two drafts above.
- Look-alike families left in sections 4-5: 101aa080/101aaa20/101a9670 (+101a8e60), 101713f0/10170580/10187520,
  10167440/10167d90, 10199e20/1019a6b0, 1009a690/10099b30, 100fda40/100fc4a0.

## Next steps
1. Node::BuildMesh: find the spelling that gives `[index*0x74 + packs]` (e.g. a VertexPack helper method, a pack
   pointer table, the index computed as a byte offset in an inline accessor); it is otherwise byte-identical.
2. Node::QueueDraw: the register-allocation residue (wall-identifier, then permute only if it is register-only);
   then its twin 1019a6b0 from the same source.
3. Other section 4-5 look-alike families (10167440/10167d90, 101713f0/10170580/10187520, 1009a690/10099b30,
   100fda40/100fc4a0): decode with `asm.py`, write in new rule-clean files.
4. The region prologue only with a genuinely new idea; npc_greet only with a new idea about Matrix4's union.

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
Work through HANDOFF 'Next steps': first the operand-order residue of Node::BuildMesh 0x101a82d0
(node_mesh_build.cpp), then Node::QueueDraw 0x10199e20's register allocation and its twin 1019a6b0, then the other
section 4-5 look-alike families in new rule-clean files (decode the frame from `asm.py`, write, fix the stack layout
with scopes and fresh locals as NOTES describes). Put every new type and global in a shared header so new matches
count under the rules. After each batch run `python tools/build_all.py --report ../progress/jp/report.json` and
`python tools/check_matches.py` from `decomp/`; commit and push only with no real loss (a rename shows as LOST + new
at the same address: update `obj_symbols.tsv`). Keep everything human-readable (named constants, liberal comments,
informative names). Work autonomously. Rewrite HANDOFF.md at the end with the Fully linked / Matched / Matched under
the rules / Fuzzy numbers, keeping its Standing instructions section and a continuation prompt for the next task,
then end your turn."
