# EN session handoff

The matching decompilation of Gurumin's English Steam exe (`re/bin/game_en_steam.exe`, VS2013 Update 4). Rewrite this
file at the end of every batch (state, not history). The next session reads it, then `../AGENTS.md`, then
`NOTES.md` (EN findings) and only the parts of `../NOTES.md` it needs. The JP run (`decomp/`) is a separate
session in the same checkout: never edit, stage or commit `decomp/`, `tools/`, `mod/`, `docs/` or the root
`README.md`; stage only `decomp_en/` and `progress/en/` files by path.

## Standing instructions (keep this section when rewriting the file)
- **Autonomous run:** work without waiting for approval; do not stop to ask questions a sensible default answers.
  Loop nonstop without ending the turn: pick, delegate, verify, commit, push, rewrite this file, pick again.
- **Opus plans, Sonnet executes.** Each round the Opus session picks 2-3 functions from `QUEUE.md` (top of section 1,
  then section 2 smallest first, then 3, then 4) whose files do not overlap, and writes one self-contained brief
  per function: `python tools/port_jp.py <address> --brief` prints a ready brief (rules, tools, compiler, the JP
  twin's matched source, the EN callees with their JP names, the EN disassembly). Add the file to write and any
  header to extend. Background Sonnet subagents (`Agent`, model sonnet, run in background) do the matching, each on
  its own files; they report MATCH or the remaining DIFF and never commit.
- **Verify, then commit:** after the subagents report, the Opus session runs from `decomp_en/`:
  `python tools/build_all.py --report ../progress/en/report.json`, then `python tools/check_matches.py` (exit 1 =
  a LOST match: fix or revert before committing; never commit a lost match), then `python tools/gen_queue.py`;
  adds a `BATCHES.md` row; checks `git diff --stat` for line-ending flips; commits only its own paths
  (`git add decomp_en/<paths> progress/en/<paths>`), pushes, rewrites this file.
- **Hard rules** (`../AGENTS.md`): typed members, never fixed offsets; each type declared once, in
  `decomp_en/include/` (a JP header only when its layout is proven identical in EN, recorded in `NOTES.md`); each
  global declared once (`extern` in a header); no fakematches (no dummy locals, `volatile`, steering blocks, permuter
  noise, inline asm); claim "matched" only from the build's `MATCH` line; never commit a lost match.
- **Readability:** no magic numbers (named constants or enums, or a comment per value); a comment above every
  function (what it does, who calls it) and on every field; informative names (carry the JP names over; rename
  `FUN_`/`DAT_` when evidence allows). C-linkage functions get readable names plus an `obj_symbols.tsv` row
  (`_Name<TAB>address`), never `FUN_<JP address>`.
- Near-misses go to `open.tsv` (`<EN address><TAB>open<TAB><reason>`) after about five honest spellings; move on.
- Keep each file's own line endings (the `.bat` files are CRLF, the rest LF).

## Progress (2026-11-24, `progress/en/report.json`)
| Measure | Value |
|---|---|
| Matched | 151 of 2,034 game functions (9,231 of 2,080,360 bytes, 0.44 %) |
| Matched under the rules | 151 |
| Queue | 506 with a matched JP twin left in section 2; open.tsv: 2 |

## State
- Pipeline verified; batches 1-4 matched Node (29+), Vec4/Matrix4/Box, containers, AnimModel channels, camera,
  IME, engine/render (IDirect3DDevice9 vtable offsets), slot table, texcache, MMV, texture manager, mesh cache,
  model bank, Game script flags and windows, mouse, config, buffer pool, colour fades, DShow, window proc.
- Header ownership is per worker; headers live in `include/` (node.h, anim_model.h, vec4.h, texture_mgr.h,
  render.h, slot_table.h, game.h, char.h, ...). Grep `include/` before declaring any type.
- `port_jp.py` finds JP sources named `M_<address>` (fixed in batch 1).
- Worker rules for briefs: scratchpad `worker_rules.txt` pattern (rules + tools + habits); give each worker
  15-20 functions of one JP area plus the headers it owns. Resume a worker for 2-3 rounds, then start a fresh one
  (contexts pass 250K tokens by round 4).
- Not applied yet: `apply_jp_names.bat` (EN Ghidra names).
- Blocked: `Game::~Game` 0x5a1150 needs a Node destructor (0x7496d0) and an EN Object base (0x76f0e0).

## Next steps
1. Section 2 of `QUEUE.md` by JP file: scene_node (37 left), char_move (18), model_params (17, AnimModel),
   app_0003/app_0010/actor_scan, model_bank, collide_query, char_effects/attach, then the larger files.
2. Section 3 (JP twin unmatched) and 4 (EN-only) after section 2 thins out.

## Continuation prompt
"Continue the Gurumin EN (Steam) matching decompilation. cd to the repository, read `decomp_en/HANDOFF.md`
(including its Standing instructions), `AGENTS.md` and `decomp_en/NOTES.md`. Loop nonstop without ending your turn:
pick 2-3 functions from `decomp_en/QUEUE.md` on disjoint files, write a brief for each with
`python tools/port_jp.py <address> --brief` (run from `decomp_en/`), start background Sonnet subagents to match them,
then verify with `python tools/build_all.py --report ../progress/en/report.json` and `python tools/check_matches.py`,
run `python tools/gen_queue.py`, add a BATCHES.md row, commit and push only `decomp_en/` and `progress/en/` paths,
rewrite `decomp_en/HANDOFF.md` with the new numbers, and pick the next functions. Never touch `decomp/` (the JP
session works there). Keep every function readable (comments, named constants, JP names carried over) and follow the
hard rules (typed members, one declaration per type, no fakematches, never commit a lost match)."
