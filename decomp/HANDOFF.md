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
- Shared so far: `vec4.h` (Vec4, Box2), `matrix4.h`, `vec3.h`, `node.h`, `com_min.h`, `ddraw_min.h`, `d3d9_min.h`
  (device, texture, surface, LockedRect). Commits 8dd6794 (Vec3), 452759b (Node, 45 files), c44b074 (Node aliases,
  world.h uses Node), 7c30db1 (COM/DirectDraw/D3D views). No match lost; renamed symbols at the same address:
  `SetFlags`, `ClearFlags`, `RestartColourPlugs`, `AllocSaved`, `EnterTargetState`, `FreeSaved`, `ReplaceMaterial`
  (were `M_<address>`), `M_101946b0` now takes `unsigned char`. `check_matches.py` exits 0 against HEAD.
- `node.h` keeps scene_node.cpp's `M_<address>` names where no caller gave a better one; its comments say what each
  does. The pointer types it only forward-declares (`PackHeader`, `KeyPair`, `Material`, `MeshData`, `Channel`,
  `TextureRef`, `NodeExtra`, `NodeSkin`, `NodeSaved`, `GameFile`, `VisData`) are still declared per file.
- Node views left per file: `SceneNode` (node_ctor.cpp constructor, model_state.cpp: its `M_1019b630` call passes an
  int where `BlendFrames` takes a float, which changes the caller's code), `WorldNode` (world_ctor.cpp
  constructor/destructor, world_frame.cpp), `FadeNode`/`FadeParent`/`FadeOwner` (node_fade.cpp; FadeParent's `glow`
  at 0x204 lies inside `Node::matrix`). Giving Node a constructor or destructor changes every file that creates
  one, so do these with care.
- **CharRecord is not a Character view:** `DAT_10db6ee8` is the AnimModel embedded at `Character + 0x10`
  (`DAT_10db6ed8`). Merging the ~21 CharRecord copies means declaring AnimModel (anim_model.cpp; `CharModel` in
  world.h is its method list) once and putting it in `Character` as `model`.
- `src/npc_greet.cpp` still keeps its own Vec4 and CharMatrix (open.tsv 100b8220). Raw offset casts left:
  `src/listbank.cpp` lines 760, 773, 796.

## Next steps
1. `NodeSaved` and the Node pointer types (`PackHeader`, `KeyPair`, `Material`, `MeshData`, `NodeExtra`, `NodeSkin`)
   into headers (scene_node.cpp, node_sort_packs.cpp, node_colours.cpp, draw_list.cpp, node_bones.cpp hold them).
2. AnimModel into a header, then `Character::model`, then the CharRecord copies (offset + 0x10).
3. The DirectInput views (`InputDevice`, `InputEffect`, `Input`: input_font.cpp, input_poll.cpp), then `NpcState`,
   `TextureObject`, `ModelBank`, and the remaining Node views above.
4. Then return to `QUEUE.md`.

## How the merges were run (reuse)
- One Python driver per type, kept outside the repo (the Node one is `node_merge_driver.py` in the session notes
  folder; copy and adapt it). For each file it builds a baseline, removes the local struct block (plus the comment
  lines directly above it), writes `#include "<type>.h"` where the struct stood, renames the view's fields and
  methods to the header's names (matched by offset and by address), rebuilds with `cmd /c .uild.bat <file>`, adds
  `obj_symbols.tsv` rows for renamed definitions, and compares matched addresses; on a loss or a compile error it
  runs `git checkout -- <file>` and logs why. Three copies run in parallel on disjoint file lists. Node: 41 of 45
  files converted unattended, the other four took a few minutes by hand.
- Process each file as bytes and keep each line's own ending: some files mix CRLF and LF (char_move.cpp,
  actor_scan.cpp), and normalising them rewrites the whole file. Check `git diff --stat` before committing.
- Rename bare field names only in files that define the type's methods, and expect a field name to also be a
  parameter name there (`frames` became `frameMatrices` in a parameter list once).
- A view whose type a shared header references (world.h's `RootNode *`) must move in the same step as the header.
- `check_matches.py` reports a renamed symbol as LOST + new. If the address and the count are unchanged, it is a
  rename; commit, then `check_matches.py` exits 0 against the new HEAD.

## Continuation prompt
"Continue the Gurumin Phase 0 type merge. Read `decomp/HANDOFF.md` (including its Standing instructions) and
`AGENTS.md`. Move `NodeSaved` and the pointer types node.h forward-declares (`PackHeader`, `KeyPair`, `Material`,
`MeshData`, `NodeExtra`, `NodeSkin`) into shared headers, then declare AnimModel once and make it `Character::model`
so the CharRecord copies (AnimModel at Character + 0x10) can go; then the DirectInput views. Convert files with a
per-file baseline/convert/rebuild/revert-on-loss driver as described under 'How the merges were run' (byte-level,
keep each line's ending) and use Sonnet subagents only for files the driver reverts. After each batch run
`python tools/build_all.py --report ../progress/jp/report.json` and `python tools/check_matches.py` from `decomp/`;
commit and push only with no real loss (renames at the same address are fine). Keep everything human-readable
(named constants instead of magic numbers, liberal comments, informative names). Rewrite HANDOFF.md at the end with
the Fully linked / Matched / Fuzzy numbers, keeping its Standing instructions section and a continuation prompt for
the next task."
