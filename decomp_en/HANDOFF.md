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

## Progress (2026-11-25, `progress/en/report.json`)
| Measure | Value |
|---|---|
| Matched | 1,004 of 2,006 game functions (202,051 of 2,078,797 bytes, 9.72 %) |
| Matched under the rules | 1,004 |
| Queue | see QUEUE.md header (after batch 15); open.tsv ~160 (incl. library/blocked rows, which leave the queue) near-misses |

## State
- Batches 1-11 matched Node, Vec4/Matrix4/Box, containers, AnimModel, camera, IME, engine/render, texture manager,
  mesh/model bank (incl. lighting, shadows), Game script, mouse, config, MMV, GameFile, sound engine (0x7be000-0x7c6220,
  ported from Ghidra + EN bytes), Steam cloud/user/text input, matrix builders, enemy AI, screen fade, menus.
- **Gate discipline:** read the build's PROGRESS/RULES lines and check_matches output BEFORE staging; stop on any LOST
  that is not a verified rename. Commit e26943f was pushed with a report from a worker's mid-edit header breakage
  (555, 83 LOST); 63004d4 repaired it. Chain the commit with `&&` after the gate, never `;`.
- Headers live in `include/`, one owner per worker; ADDITIVE edits by anyone via the Edit tool, then rebuild the
  includers. Grep `include/` for a type or an EN data address before declaring it.
- Worker rules: scratchpad `worker_rules.txt` (rules, tools, habits, checks before reporting). Give each worker 15-20
  functions of one area; resume a worker for 2-3 rounds, then start a fresh one (contexts grow past 250K).
- Workers never `git stash`/reset/checkout/switch; never sed -i over shared files; backslash strings only via Edit/Write.
- Push: plain push usually works now (CI no longer commits meters). Fallback: scratchpad `push_onto_remote.sh`
  (temporary-index rebuild on origin/master, then soft reset).
- Check `git diff --stat` before each commit: a worker's sed -i turned obj_symbols.tsv into mixed CRLF/LF in batch 12
  (normalised back to LF); batch 13 flipped five more. Before committing run
  `git ls-files --eol -- decomp_en` and restore any file whose w/ differs from i/. Open puzzle: CreateShortcut calling convention (NOTES).
- Lever to test: `Game::TestScriptFlag` is inline in game.h, but the original calls it out of line (0x5ec240) from
  DamageZone_Update 0x63ac70 and Route_FollowGates 0x5ee110; some callers may inline it. Also `small_b11.h` is a
  catch-all header from batch 11: split its declarations into area headers when convenient.
- Type puns to clean up (batch 15): DShowStream::EndStreams casts to `IDirectDrawMediaStreamLike *` for vtable slot 8
  (declare slot 8 on the interface instead); Packs_DecodePlaneBands casts between two band views (make one type).
- PR 3 (EN launcher, MFC) is merged: `E:\Microsoft Visual Studio 12.0` is now VS2013 Community Update 4 with
  `VC\atlmfc`; 28 MFC/ATL instantiations reclassified as library (2,006 game functions). The launcher range
  0x6e5d60-0x6f15af is open again; open.tsv lists what PR 3 left. Before each push: `git fetch`, merge origin/master
  if it moved (TSV/MD conflicts: keep both sides, then check obj_symbols for duplicate rows).
- Never add address-taking tables or macros that only force COMDAT emission.
- Blocked: `Game::~Game` 0x5a1150 needs Node dtor + EN Object base.
- Batch 11b (701) committed all three batch-11 workers. EN-only: CRT math wrappers, string.h overloads and Steam SDK
  CCallback slots are library code (skip); many small ctors/dtors wait for their class layouts. Lists still open in the
  scratchpad: list_sec2b, rest of list_sec3b, list_sec3c, list_sec4.

## Next steps
0. Yield per batch: EN-only small functions ~90 %, section 3 ~45 %, section 2 (large) <20 %. Favour EN-only and
   section 3; record unknown-layout and library functions in open.tsv with status `blocked`/`library`.
1. Section 2 rest (99, large): FontRandom_Load, File_Checksum, then by JP file.
2. Section 3 small/medium functions (JP twin unmatched; port from Ghidra + EN bytes).
3. Section 4 EN-only outside the launcher.

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
