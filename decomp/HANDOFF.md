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
- Shared headers: `vec4.h`, `matrix4.h`, `vec3.h`, `node.h`, `com_min.h`, `ddraw_min.h`, `d3d9_min.h`, and new this
  session `key_track.h` (KeyTrack, KeyBank), `node_data.h` (NodeExtra, NodeSkin, NodeSaved), `mesh.h` (Vertex,
  VertexPack, IndexList, Material, MeshData), `anim_model.h` (AnimModel, MotionChannels, Playback, `DAT_10db6ee8`),
  `dinput_min.h` (DirectInput views, Input `DAT_10f08008`). Commits 930f50c, 768cc82, b7ce8fd, b279ff9, 6669791.
  No match lost; renamed symbols sit at the same addresses (rows in `obj_symbols.tsv`).
- **AnimModel is 0x249c bytes = the record stride.** The old world.h `Character` (`DAT_10db6ed8`) was the same array
  seen 0x10 bytes early; world.h no longer declares it and every user indexes `DAT_10db6ee8` (offset - 0x10). The
  record id is `DAT_10db6ee8[i - 1].nextSlotId` (NOTES.md 2026-11-22 section).
- Deferred: `src/npc_greet.cpp` keeps its own Vec4/CharMatrix/Character view (open.tsv 100b8220: the shared Vec4
  tips register allocation). `model_state.cpp` (SceneOwner/SceneNode views, defines Playback methods),
  `mesh_cache.cpp` LayerTable (= Playback layer setters), duplicate `DAT_10faaa88` externs. Node views with
  constructors (`SceneNode`, `WorldNode`, `FadeNode`/`FadeParent`/`FadeOwner`) still per file. Raw offset casts:
  `src/listbank.cpp` lines 760, 773, 796.
- Many AnimModel methods keep `M_<address>` names and many fields `m_XXXX`; name them when evidence appears.

## Next steps
1. `NpcState`, `TextureObject` (7 views), `ModelBank`/`BankModel`, `EffectManager`, `ListBank`, `Game` into shared
   headers (BACKLOG Phase 0 item 2 and Phase E item 27), same driver method.
2. model_state.cpp / mesh_cache.cpp LayerTable onto `Playback`; the remaining Node views with constructors.
3. Move the `extern "C"` globals next to their owner types (BACKLOG Phase 0 item 3).
4. Then return to `QUEUE.md`.

## How the merges were run (reuse)
- One Python driver per type, kept outside the repo (Node: `node_merge_driver.py`; AnimModel: `am_driver.py`; Character: `ch_driver.py`, all in the session
  notes folder; copy and adapt them). For each file it builds a baseline, removes the local struct block (plus the comment
  lines directly above it), writes `#include "<type>.h"` where the struct stood, renames the view's fields and
  methods to the header's names (matched by offset and by address), rebuilds with `cmd /c <absolute path to build.bat> <file>`, adds
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
`AGENTS.md`. Move `NpcState`, `TextureObject`, `ModelBank`/`BankModel`, `EffectManager`, `ListBank` and `Game` into
shared headers under decomp/include/ (one declaration each, typed members), then put model_state.cpp and
mesh_cache.cpp's LayerTable onto `Playback` (anim_model.h). Convert files with a per-file
baseline/convert/rebuild/revert-on-loss driver as described under 'How the merges were run' (byte-level, keep each
line's ending; take the baseline from a real build, not an empty list) and use Sonnet subagents only for files the
driver reverts. After each batch run `python tools/build_all.py --report ../progress/jp/report.json` and
`python tools/check_matches.py` from `decomp/`; commit and push only with no real loss (renames at the same address
are fine). Keep everything human-readable (named constants instead of magic numbers, liberal comments, informative
names). Rewrite HANDOFF.md at the end with the Fully linked / Matched / Fuzzy numbers, keeping its Standing
instructions section and a continuation prompt for the next task."
