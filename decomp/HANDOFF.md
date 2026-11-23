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

Under AGENTS.md rules 1-2 most matches are still MATCH-PENDING until their other types and globals move into shared
headers (Phase 0 of `BACKLOG.md`; step 2 is done, step 3 is next).

## State
- Shared headers: `vec4.h`, `matrix4.h`, `vec3.h`, `node.h`, `node_data.h`, `mesh.h`, `key_track.h`, `anim_model.h`,
  `com_min.h`, `ddraw_min.h`, `d3d9_min.h`, `dinput_min.h`, `char_state.h`, `model_bank.h`, `game.h`, `listbank.h`,
  and changed or new this session (a0c15e0..b115cec, 789 matched unchanged throughout):
  - `texture_object.h`: TextureObject now derives from `TextureHandle` (texture, surface; ctor 0x10143bb0, was
    IntPair in engine_10143.cpp). mesh_cache.cpp's MeshInstance is gone: Create/Unload/Recreate/Restore/UpdateImage
    and the constructor 0x10169c40 are TextureObject methods; source image fields typed (sourcePixels, sourceAlpha,
    hasOwnAlpha, generatedAlpha) with ALPHA_* constants.
  - `effect_manager.h`: also EffectDef (DAT_1101dfdc), EffectRecord (DAT_1115cfe8; `partPositions[2]` indexed by
    `part`, which is only ever 0 or 1), EffectSprite (both sprite tables), EffectLight, the list heads; EffectOptions'
    m_0010 has a union exposing `drawn` (record byte 0x1a6). Views gone from char_effects, effect_start,
    effect_defs, effect_sprites, slot_alloc.
  - `char_slot.h` (new; `char_params.h` removed): CharSlot = the 0x188-byte character record at **DAT_10daaae8**
    (the save copies 0x188 bytes from there) plus the template DAT_10db4408, in CharParams' layout (int arrays
    stats[16] / m_0044[16] / params[16], single fields 0x84..0xec, ParamVector position: the implicit copy
    assignment 0x100357a0 depends on it). CharStat / CharParam enums name the array slots used so far. The old
    CharSlot views of DAT_10daab84 (0x9c late), ActorSlot/ActorFace of DAT_10daaaec (4 late), SlotRecord, KindSlot,
    PlayerRecord and the save files' char arrays are gone. world.h includes it.
  - `char_state.h`: the `words[k]` users have names (cameraFrames, walkingBack, hideCount, downTimer, downMode,
    reviveHealth, keepCompanionUp, companionHealthCap, weaponMarkA/B). Left by index on purpose: enemy_voice.cpp
    (`words[slot]`, a variable index) and the block resets in char_kind_select.cpp / char_reset.cpp.
  - `tools/merge_configs/char_slot.py` is the config of the ActorSlot pass.
- Deferred / open:
  - More aliases of the CharSlot table still declared per file as their own globals: `DAT_10daadfc`, `DAT_10daae00`,
    `DAT_10daae0c`, `DAT_10daaaf0`, `DAT_10daaafc` (`int [0x80][0x62]`), `DAT_10daaf39`/`DAT_10daaf3a`
    (`char [0x80][0x188]`), `DAT_10daaaf8`, `DAT_10daab04`, `DAT_10daab20` (CharSlotFlags), `DAT_10daabb4`,
    `DAT_10daabb8`, `DAT_10daac2d`, CharLife views. Each is DAT_10daaae8[i (+2 for the companion copies)].field;
    rewrite them as CharSlot fields (relocation targets may change symbol: build.bat still matches, as seen this
    session). companion_update.cpp uses most of them.
  - ListBank::ReportError was inline in the original (listbank_vars.cpp expanded one call); the header keeps it out of
    line because listbank.cpp's 0x100359c0 copy is matched.
  - Playback::Play (unmatched) passes BlendFrames' third argument as an int; node.h says `float t`.
  - DAT_10874848 and DAT_104b7200 both behave as Game but lie 0x3bd648 bytes apart (less than sizeof(Game)).
  - `src/npc_greet.cpp` own Vec4/CharMatrix/Character view (open.tsv 100b8220); Node views with constructors
    (SceneNode, WorldNode, FadeNode/FadeParent/FadeOwner); `ParamVector` may be Vec4 with operator= = Copy.

## Next steps
1. BACKLOG Phase 0 step 3: move the `extern "C"` globals next to their owner types, one declaration each, starting
   with the CharSlot aliases above (they are fields, not globals), then the other per-file `extern "C"` lines
   (group them by owner header; a global with no owner type gets a header per subsystem).
2. BACKLOG Phase 0 step 4: teach `build_all.py` to report "matched under the rules".
3. Then return to `QUEUE.md`.

## How the merges were run (reuse)
- Generic driver `tools/merge_driver.py <config.py> <tag> <files>` (usage in its docstring; configs in
  `tools/merge_configs/`). Per file it removes the view structs, drops the global's extern lines, adds the header
  include, renames types and fields through roots, rebuilds, and reverts on a loss or compile error (baseline = HEAD's
  report.json by address). Small merges (a handful of files) are quicker as a byte-level Python rewrite plus one
  `build_all.py`.
- Process files as bytes and keep each line's ending; never `sed -i` a CRLF file (it flipped char_dispatch.cpp
  again this session; fixed before commit). `obj_symbols.tsv` is LF. Check `git diff --stat` before committing.
- `check_matches.py` reports a renamed symbol as LOST + new. If the address and the count are unchanged, it is a
  rename; update the `obj_symbols.tsv` row (a missing row shows as the function dropping to 0% fuzzy), commit, then
  `check_matches.py` exits 0 against the new HEAD. Compare fuzzy per address against HEAD as well.

## Continuation prompt
"Continue the Gurumin Phase 0 type merge. Read `decomp/HANDOFF.md` (including its Standing instructions) and
`AGENTS.md`. Do BACKLOG Phase 0 step 3: rewrite the per-file aliases of the CharSlot table (DAT_10daadfc,
DAT_10daaf39, DAT_10daab20, ... listed under 'Deferred / open') as fields of `include/char_slot.h`'s CharSlot at
DAT_10daaae8, then move the remaining `extern "C"` globals into headers next to their owner types (one
declaration each). Convert files with `decomp/tools/merge_driver.py` or byte-level Python rewrites (keep each
line's ending; baseline from HEAD's report) and use Sonnet subagents only for files that resist. After each batch
run `python tools/build_all.py --report ../progress/jp/report.json` and `python tools/check_matches.py` from
`decomp/`; commit and push only with no real loss (renames at the same address are fine). Keep everything
human-readable (named constants instead of magic numbers, liberal comments, informative names). Rewrite HANDOFF.md at
the end with the Fully linked / Matched / Fuzzy numbers, keeping its Standing instructions section and a continuation
prompt for the next task (when step 3 is done: BACKLOG Phase 0 step 4, then back to `decomp/QUEUE.md` matching)."
