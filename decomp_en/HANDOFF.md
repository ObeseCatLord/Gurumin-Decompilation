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
| Matched | 2 of 2,034 game functions (94 of 2,080,360 bytes, 0.00 %) |
| Matched under the rules | 2 |
| Queue | 655 with a matched JP twin, 486 mapped to an unmatched JP twin, 891 EN-only |

## State
- Pipeline ready and verified (batch 0): toolchain (`env.bat`, `build.bat`, `tools/run_cl.py` for the UTF-8 code
  page), flags `/O2 /Oy- /GS- /EHsc` (NOTES "Compile flags"), classification (`classify.tsv`), JP->EN map
  (`jp_en_map.tsv`, 1,143 of 2,034 mapped), queue, report and gate. `Node::Find` and `Map_RunFrameScript` match.
- `ghidra/ApplyJpNames.java` (via `apply_jp_names.bat`) was verified in a read-only dry run (1,115 names); it has
  not been applied to the saved EN Ghidra project yet. Applying it renames FUN_ functions in
  `re/export/game_en_steam.exe/`; the tools do not depend on those names (classify keeps a fixed FID list).
- `flag_tests/` holds the six flag-test functions (not built by build_all; they declare local types).

## Efficiency over the JP workflow
1. **Port before briefing:** `tools/port_jp.py --brief` assembles the brief mechanically (JP matched source, callee
   names, disassembly); 657 EN functions have a matched JP twin, and both first ports matched unchanged.
2. **Batch small ports:** for section-2 functions under ~150 bytes, give one subagent 5-10 functions that share a JP
   source file (same EN file name), instead of one function per brief.
3. **Proposed next tool:** `tools/port_file.py <jp file.cpp>` that copies a whole matched JP file into `src/`,
   rewrites every JP address (function names, `obj_symbols` rows, globals) to its EN twin through `jp_en_map.tsv`,
   builds it and keeps the functions that print MATCH; the rest become section-1 drafts. Most of section 2 could
   then be cleared without a model in the loop, leaving models for the residues.
4. **Headers grow by proof:** when an EN match uses a JP struct's fields at the JP offsets, move that JP header's
   declarations into `include/` wholesale (record it in NOTES) instead of re-deriving fields one by one.

## Next steps
1. Section 2 of `QUEUE.md` (JP twin matched), smallest first; start with the camera setters, Vec4/Matrix4 helpers and
   Node methods (shared headers grow fastest there).
2. Consider writing `tools/port_file.py` (efficiency item 3) before the large-function part of section 2.
3. Apply the JP names to the EN Ghidra project (`apply_jp_names.bat`) when no other session is using it.

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
