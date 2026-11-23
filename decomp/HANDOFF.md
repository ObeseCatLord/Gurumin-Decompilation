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
| Matched under the rules | 6.19% of code (106538 bytes), 488 of the 789 matched functions |
| Fuzzy match | 14.66% |

"Matched under the rules" (BACKLOG Phase 0 step 4, done): a matched function counts only when its file declares no
struct/class/union and no `extern` global of its own (`tools/rule_check.py`; build_all prints a `RULES` line and
report.json carries `matched_rules_code`, `_code_percent`, `_functions`, `_functions_percent` in every measures block).
`python tools/rule_check.py` lists the 117 files still breaking rule 2 (301 matched functions), most matches first.

## State
- This session (c7daa9a..ab37641, 789 matched and every fuzzy value unchanged throughout):
  - New tools: `rule_check.py`; `type_move_driver.py plan|apply [files]` (moves a file's file-scope types, with the
    `#define`s and callback typedefs they use, into include/<type>.h or include/<file>.h, builds each file and reverts
    losers; identical copies across files share one header); `prune_header_includes.py <h>...` (drops includes a
    header does not need alone); `use_header_globals.py <header> <globals>` (replaces per-file externs by an include);
    `resolve_globals.py <header> <globals>` (tries each file's declaration of a conflicting global as the shared one).
  - About 60 new headers (type headers named after the type or the first file using it), `engine_memory.h`
    (allocate/free/copy pointers), `win_vars.h` (GUIDs, HWND, HINSTANCE, tick counts), globals added to the
    `*_vars.h` / `char_tables.h` headers and to their owner type's header (ime.h, item_info.h, tracker.h).
  - Views merged: GameFile (game_file.h; `Open` returns the FILE *, obj_symbols row renamed), ItemInfo (item_info.h,
    with the item table DAT_1088e6b0), TextureManager, MeshSlot, Tracker, StartVectors, ScreenVertex, TextureRef
    (mesh_cache's `index` is the slot), Channel (its `keys` is a MotionList: Clear/InsertByName), Plane. The overlay
    model DAT_104c03c8 is a Node (QueueDraw/Release), render_vars.h.
  - `globals_driver.py` now ignores npc_greet.cpp when planning (it had written that file's globals into a second
    header). Its loss check only looks at the files it rewrote: always run build_all + check_matches after it.
- Deferred / open (what keeps the other 301 matches pending):
  - **Struct views in several files** (need a field union and renames per file): CollisionWorld (11 files),
    LastPosition (10), TextRenderer (5), Engine (4), WorldRenderer (4; WorldRenderer::Bind takes ModelRecord =
    AnimModel, so its mangled name changes), MapObject (3), MmvPlayer (3; MoviePlayer in app_0014/player_damage is
    MmvPlayer too, but M_10160960 takes ClipRect* in mmv_texreg and TextureObject* elsewhere), TriangleBatch (3),
    MapEntry, ModelRecord (= AnimModel: m_0265 important, node = headNode), WorldNode, Object (game.h has another
    Object), Channel in node_world.cpp (its Keys::M_101ad350 is MotionList::FrameAt but is called with a float).
    scene_misc.cpp and window_create.cpp keep their types (they use the per-file Object/WindowCallback).
  - **Globals the files read as different types** (`resolve_globals.py` found no declaration that fits every file):
    DAT_10255f08 (22 files: CollisionWorld*/Node*/void*), DAT_10802bf8 (15), DAT_10255160 (8), DAT_1021bdf8 (7),
    DAT_103150c0 (7), DAT_10836c70 (6), DAT_10f6550c/DAT_10f65510 (int sentinel 1 vs TextureObject*), DAT_1101d884
    (CameraTarget* vs int) and about 70 more (`python tools/globals_driver.py plan` lists 72 with reasons). Settling one
    means choosing the true type and rewriting the other files' uses readably (casts only where the original did).
  - GameFile is 0x28 bytes in game_file.h but MovieFile (mmv_texreg.h, the same engine object inside MmvPlayer) is
    0x24; the 0x28 came from a stack frame, so 0x24 is probably right (check before merging them).
  - npc_greet.cpp stays skipped (open.tsv 100b8220). 0x10023900 fuzzy moves 95.4-95.8 with header contents.
  - Older open items: ListBank::ReportError inline in the original; Playback::Play passes BlendFrames' third
    argument as an int; DAT_10874848 and DAT_104b7200 both behave as Game; ParamVector may be Vec4.
- QUEUE.md: sections 2-3 (0x1000cf80, 0x10017c55, 0x10161781, ...) are mis-split pieces of larger functions
  (0x1000cf80 jumps past its own ret); real matching work is the walls (section 1) and the large functions (section 4).

## Next steps
1. Return to `QUEUE.md` matching: the walls first only where a reason looks fixable under cl 13.00, then section 4
   (start with 0x1014f590, 0x10055ee0, 0x100d2110). New code puts its types and globals in headers from the start
   (include the `*_vars.h` / type header instead of writing an extern), so every new match counts under the rules.
2. When a matched file is touched anyway, merge its remaining view (list above) and re-run `rule_check.py`.

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
Phase 0 step 4 is done (build_all reports 'matched under the rules'). Return to `decomp/QUEUE.md` matching: walls
whose reason looks fixable first, then section 4's large functions; put every new type and global in a shared header
so new matches count under the rules, and merge a file's remaining struct views (HANDOFF 'Deferred / open') when you
touch it. After each batch run `python tools/build_all.py --report ../progress/jp/report.json` and
`python tools/check_matches.py` from `decomp/`; commit and push only with no real loss. Keep everything
human-readable (named constants, liberal comments, informative names). Work autonomously. Rewrite HANDOFF.md at the
end with the Fully linked / Matched / Matched under the rules / Fuzzy numbers, keeping its Standing instructions
section and a continuation prompt for the next task, then end your turn."
