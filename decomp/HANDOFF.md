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
- Shared headers: `vec4.h`, `matrix4.h`, `vec3.h`, `node.h`, `node_data.h`, `mesh.h`, `key_track.h`,
  `anim_model.h` (AnimModel, Playback, DAT_10db6ee8, model table DAT_10faaa88), `com_min.h`, `ddraw_min.h`,
  `d3d9_min.h`, `dinput_min.h`, and new this session (commits f3fbdfc..6934e47, 789 matched unchanged throughout):
  - `char_state.h`: CharState = the 0x40-byte behaviour state of each character slot (DAT_1020b430); every offset
    is a union of the names behaviours use (NpcState, AiState, ScriptState, ... views gone), `values[13]` by index,
    `words[16]`, `shoves[4]` (Shove). world.h includes it.
  - `model_bank.h`: ModelBank (DAT_10804bf8); its records are AnimModels (BankModel gone; AnimModel gained
    uniformScale, bankIndex, shadowHidden, loadedRoot, shadowOpacity, shadowMatrix and `~AnimModel()`).
  - `effect_manager.h`: EffectManager (DAT_10daaa80) and EffectOptions (was ModelState; ctor 0x101668b0 `throw()`,
    byte settings are `char settings[8]`, see NOTES).
  - `texture_object.h`: TextureObject and the 1600 texture slots DAT_118d0ba8 (RenderItem/TextureSlot gone).
  - `game.h`: now also declares DAT_104b7200 and DAT_10874848; per-file Game views gone.
  - `listbank.h`: ListBank (DAT_104687b8; ResourceBanks, TokenReader, ScriptCompiler views gone), with the
    `ScriptVariableValue` union and `resultBuffer`.
  - model_state.cpp / mesh_cache.cpp now define Playback methods on anim_model.h's Playback.
- Deferred / open (next candidates, each a view still declared per file or a known wrinkle):
  - Whole-block CharState users still index `words[k]` (camera_follow, char_kind_select, companion_update,
    route_follow, weapon_attach, enemy_voice): give each index its behaviour's field name.
  - `mesh_cache.cpp` MeshInstance is a TextureObject view (MeshInstance::M_10169f70 = TextureObject::UpdateImage;
    m_002c/m_0030 = sourceWidth/sourceHeight).
  - Effect records DAT_1115cfe8: `EffectRecord` (effect_start.cpp) and `EffectRecordA` (char_effects.cpp).
  - CharSlot views (enemy_voice.cpp vs world.h), ActorSlot views (npc_frozen.cpp and others, DAT_10daaaec).
  - ListBank::ReportError was inline in the original (listbank_vars.cpp expanded one call); the header keeps it
    out of line because listbank.cpp's 0x100359c0 copy is matched. listbank_vars' two functions are unmatched.
  - Playback::Play (unmatched) passes BlendFrames' third argument as an int; node.h says `float t` (fuzzy 18.8 -> 15.1).
  - DAT_10874848 and DAT_104b7200 both behave as Game but lie 0x3bd648 bytes apart (less than sizeof(Game)).
  - From before: `src/npc_greet.cpp` own Vec4/CharMatrix/Character view (open.tsv 100b8220); Node views with
    constructors (SceneNode, WorldNode, FadeNode/FadeParent/FadeOwner); `extern "C"` globals next to their owners.

## Next steps
1. The deferred views above (MeshInstance, EffectRecord/EffectRecordA, CharSlot/ActorSlot, the CharState
   `words[k]` users), same driver method.
2. Move the remaining `extern "C"` globals next to their owner types (BACKLOG Phase 0 item 3).
3. Then return to `QUEUE.md`.

## How the merges were run (reuse)
- Generic driver `tm.py <config.py> <tag> <files>` in the session notes folder
  (`C:\Users\Michael\AppData\Local\Claude-Data\claude_pzs8sxrjxfjjc\side-session-notes\local_93c4d5c5-8746-4eea-a63d-32135252eefa`; configs `cfg_charstate.py`,
  `cfg_modelbank.py`, `cfg_effect.py`, `cfg_tex.py`, `cfg_game.py`; `lb_convert.py` for ListBank). Per file it removes
  the view structs (with the comment lines above), drops the global's extern lines, adds the header include (after
  the leading includes), renames types, renames fields only through roots (the global with its indexes, or pointers
  declared with a view type), applies per-file raw regexes, rebuilds, and reverts on a loss or compile error; the
  baseline is HEAD's report.json by address. When a C++ symbol vanished it adds obj_symbols rows for the same
  `?Method@Class@@` at the old address. Three copies run in parallel on disjoint file lists.
- Process files as bytes and keep each line's ending; never `sed -i` a CRLF file (it rewrote anim_reset.cpp once).
  `obj_symbols.tsv` is LF. Check `git diff --stat` before committing.
- `check_matches.py` reports a renamed symbol as LOST + new. If the address and the count are unchanged, it is a
  rename; commit, then `check_matches.py` exits 0 against the new HEAD. Also compare fuzzy per address against
  HEAD: a missing obj_symbols row shows as a function dropping to 0%.

## Continuation prompt
"Continue the Gurumin Phase 0 type merge. Read `decomp/HANDOFF.md` (including its Standing instructions) and
`AGENTS.md`. Merge the remaining per-file views listed under 'Deferred / open' into shared headers under
decomp/include/ (one declaration each, typed members): mesh_cache.cpp's MeshInstance onto TextureObject, the effect
records (EffectRecord/EffectRecordA, DAT_1115cfe8) into effect_manager.h, CharSlot and ActorSlot, and give the
CharState `words[k]` users named fields. Convert files with the generic driver `tm.py` from the session notes folder
(byte-level, keep each line's ending; baseline from HEAD's report) and use Sonnet subagents only for files the driver
reverts. After each batch run `python tools/build_all.py --report ../progress/jp/report.json` and
`python tools/check_matches.py` from `decomp/`; commit and push only with no real loss (renames at the same address
are fine). Keep everything human-readable (named constants instead of magic numbers, liberal comments, informative
names). Rewrite HANDOFF.md at the end with the Fully linked / Matched / Fuzzy numbers, keeping its Standing
instructions section and a continuation prompt for the next task."
