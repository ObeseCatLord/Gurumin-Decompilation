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
| Measure | Value |
|---|---|
| Fully linked | 0.00% of code (151 of 264 units complete) |
| Matched | 13.94% of code, 820 of 1212 functions |
| Matched under the rules | 13.90% of code, 819 of the 820 matched functions |
| Fuzzy match | 18.44% |

"Matched under the rules": a matched function counts only when its file declares no struct/class/union and no
`extern` global of its own (`tools/rule_check.py`; build_all prints a `RULES` line). Only npc_greet.cpp (1 matched
function) and node_world.cpp (0) still break rule 2.

## State
- **Workflow (parent session, 2026-11-23):** per-function matching goes to Sonnet subagents (2-3 in parallel on
  disjoint files) with a self-contained brief (tools, idioms, hard rules, "report: matched or bytes off, do not
  commit"); the Opus session picks functions, verifies with build_all + check_matches, commits, pushes, updates this.
- Batches 310-311 matched Watcher_Update 0x10026730 (hover_watcher.cpp), Map_RoutePosition 0x10049b00
  (map_route.cpp: points at 2i/2i+1, 1-based marks[], three locals sharing one slot) and Enemy_BackAwayFromPlayer
  0x100279b0 (enemy_flee.cpp + enemy_flee.h). MapCursor_Step 0x10051190 is a draft (map_cursor.cpp, open.tsv).
- Batch 312: Debris_Draw 0x1000e340 (debris.cpp + debris.h; Node::StartProximityPass takes Matrix4 *reference),
  Char_ProbeSideStep 0x100509e0 and Char_ProbeSideLedge 0x10050df0 (ledge_probe.cpp) matched; Playback::Advance
  0x1016a480 drafted (playback_advance.cpp, shared tails; open.tsv).
- Batch 313: Message_DrawPanel 0x1000eed0 (message_panel.cpp), Characters_Draw 0x1000c720 (character_draw.cpp) and
  FontRenderer::DrawTextClipped 0x1014fdd0 (font_metrics.cpp) matched; DrawStringToPixels 0x1014f900 (2 bytes),
  KeyBank::Sample(float) 0x101af960 and KeyTrack::Sample 0x101afe00 drafted. Lead: the three KeyBank/KeyTrack
  samplers may need Spline as a thiscall member (vec4.cpp). ime_draw.cpp should call DrawTextClipped as a method.
- Batch 314: Camera_LiftOverGround 0x1016be80 (camera_lift.cpp; DAT_1101dde4 is a float) matched; Spline is now
  KeyInterpolator::Spline (empty base of KeyBank/KeyTrack), the sampler call blocks match, other residues remain;
  Player_SelectActionMotion 0x10011520 drafted (action_select.cpp, count kept in a dead stack slot).
- Batch 315: ListBank::DeclareFromText 0x100ed510 (listbank.cpp) and Lava_Update 0x1005f750 (lava_hazard.cpp, new
  effect_holds.h) matched; Regions_Update 0x1004d4b0 drafted (region_view.cpp, zero register).
- **enemy_barrage.cpp + barrage_enemy.h** (new, rule-clean, C++ linkage because the originals have EH frames):
  Enemy_CallBarrage 0x100a77f0 (behaviour 15), Enemy_SpinFire 0x100a7db0 (14) and Enemy_CallBursts 0x100a8370
  (16/17, variant argument) match. Enemy_SpinFire's lesson (NOTES): a node pointer with a fallback (`fire`, else the
  head) is two cross-jumped calls `if (a) Call(a); else if (b) Call(b); else {...}`, not `a || (a = b)`.
- **Enemy_StarVolley 0x100a7150** (13) and **Enemy_AimedShots 0x100a6a50** (12) drafted there: control flow, calls
  and frame match; only pairs of 4-byte stack slots are swapped (open.tsv lists the spellings tried; AimedShots gets
  to 5 bytes by reusing dx for the turn). The ring points are `DAT_10868838[index][n - 1]` (the `lea [eax-0x40]`
  displacement is inside the masked relocation).
- **Enemy_FlyAndStrafe 0x1009c070** (clamp temporaries), Enemy_StrafeAndShoot 0x1009a690 / Enemy_StrafeAndBurst
  0x10099b30 (ebx/edi swap), Enemy_ChaseAndSlash 0x1009d490 (side-step tails), DrawPictures 0x10167440,
  Node::QueueDraw 0x10199e20, Node::BuildMesh 0x101a82d0, npc_greet.cpp, node_world.cpp and the region prologue
  wall: unchanged (see open.tsv).
- **101713f0 / 10170580 / 10187520** (trail renderers, 0x1600-byte frames, KeyBank locals built with the array
  constructor helper FUN_10004650, effect definitions at DAT_1101dfdc 0x3d8 each, point lists at DAT_118e99f8 0x4014
  each): not started; they need new types for both tables first.
- The large Enemy_UpdateBehaviour kinds 10076790 (the common enemy behaviour), 100a5960, 100a9240, 100aa820,
  100ade30, 100af1b0 are unstarted.

## Next steps
1. The trail renderer family 101713f0/10170580/10187520: type the effect definition (0x3d8) and trail point list
   (0x4014) tables first, then the functions in a new rule-clean file.
2. The large Enemy_UpdateBehaviour kinds (start with the smallest of 100a5960/100a9240/100aa820/100ade30/100af1b0;
   enemy_barrage.cpp shows the shared shapes: Vec4 player/self, atan2 facing, EffectOptions + Matrix4 temporaries).
3. Near-misses with a genuinely new idea: the stack-slot swaps of 100a7150/100a6a50 (maybe a permuter run),
   Enemy_FlyAndStrafe's clamps, 1009d490's side-step tails, the 1009a690/10099b30 swap.
4. Section 4 large functions of `decomp/QUEUE.md` in order (character behaviours run from 0x100f9670 and the
   map-route family match well: delegate them to subagents).

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
Work through HANDOFF 'Next steps': first the trail renderer family 101713f0/10170580/10187520 (type their tables
first), then the large Enemy_UpdateBehaviour kinds, then the remaining near-misses with a genuinely new idea. Put
every new type and global in a shared header so new matches count under the rules. After each batch run
`python tools/build_all.py --report ../progress/jp/report.json` and `python tools/check_matches.py` from `decomp/`;
commit and push only with no real loss (a rename shows as LOST + new at the same address: update `obj_symbols.tsv`).
Keep everything human-readable (named constants, liberal comments, informative names). Work autonomously. Rewrite
HANDOFF.md after each batch with the Fully linked / Matched / Matched under the rules / Fuzzy numbers, keeping its
Standing instructions section and a continuation prompt."
