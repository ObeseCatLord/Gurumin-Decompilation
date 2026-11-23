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
| Fully linked | 0.00% of code (147 of 251 units complete) |
| Matched | 11.62% of code (199908 of 1720206 bytes), 790 of 1212 functions |
| Matched under the rules | 6.24% of code (107403 bytes), 489 of the 790 matched functions |
| Fuzzy match | 14.78% |

"Matched under the rules": a matched function counts only when its file declares no struct/class/union and no
`extern` global of its own (`tools/rule_check.py`; build_all prints a `RULES` line). `python tools/rule_check.py`
lists the files still breaking rule 2 (301 matched functions), most matches first.

## State
- This session (1148f97, 39a5c47; QUEUE.md section 4):
  - **FontRenderer::DrawString 0x1014f590 matched** (font_metrics.cpp, rule-clean). GlyphWidth's two trailing
    arguments are `unsigned short` (the caller pushes a 16-bit register); the header's parameter names were wrong
    (it is color, x, y). Pattern: `i++` before reading a command's digits, then `i += 2/3`.
  - Player_KnockBack 0x10055ee0 (new player_knockback.cpp, rule-clean, all globals in headers): 18 bytes off, open
    (the original stores each ground-probe result as a 0/1 int in a stack slot before testing it).
  - KeyBank::Sample 0x101af4c0 (vec4.cpp) and Spline 0x101ad920 moved from containers.cpp to vec4.cpp: Sample needs
    Vec4() and Spline defined earlier in its file to keep `this` in edx (NOTES "Callee register knowledge"). Open:
    the linear flag is folded into `test al,4` and (float)offset is hoisted above the linear/spline branch.
    KeyTrack::Sample 0x101afe00, 0x101af960 and 0x101b0240 are the same code for the other containers: once one
    matches, the others follow (put them in vec4.cpp too).
  - Spline is probably a thiscall member (every caller does `mov ecx, edx`), but adding a member declaration to
    KeyBank changed FUN_100b4f00 in char_move.cpp (NOTES "Header declarations can change unrelated code").
  - Globals moved into headers: DAT_104c022c/022d/10802ad8 (render_vars.h, position history), DAT_1032d6a4 and
    DAT_104c0090 (game_vars.h); game_reset.cpp and scene_start.cpp lost their copies.
  - GlyphWidth 0x10145380 retried (six byte-swap spellings): the original loads the code with a 16-bit `mov ax`,
    every spelling here loads 32 bits; still open.
- Deferred / open (what keeps the other 301 matches pending): unchanged from the previous handoff:
  - **Struct views in several files**: CollisionWorld (11 files), LastPosition (10), TextRenderer (5), Engine (4),
    WorldRenderer (4; WorldRenderer::Bind takes ModelRecord = AnimModel), MapObject (3), MmvPlayer (3), TriangleBatch
    (3), MapEntry, ModelRecord (= AnimModel), WorldNode, Object, Channel in node_world.cpp.
  - **Globals read as different types**: DAT_10255f08 (22 files), DAT_10802bf8 (15), DAT_10255160 (8), DAT_1021bdf8
    (7), DAT_103150c0 (7), DAT_10836c70 (6), DAT_10f6550c/DAT_10f65510, DAT_1101d884 and about 70 more
    (`python tools/globals_driver.py plan`).
  - GameFile 0x28 vs MovieFile 0x24; npc_greet.cpp skipped; ListBank::ReportError inline; DAT_10874848/DAT_104b7200
    both Game; ParamVector may be Vec4.
- Section 4 candidates looked at and left: 0x100d2110 (needs DAT_1021bdf8/DAT_10255160 settled first), 0x10164c10
  (an unnamed lighting class, 0x160-byte lights at +0x1888), 0x1001bb20 (EH frame + draw-parameter struct),
  0x10049b00 and 0x10026730 (float-heavy route code), 0x100360f0 (script tokenizer with three flags).

## Next steps
1. KeyBank::Sample family (0x101af4c0, 0x101afe00, 0x101af960, 0x101b0240): try spellings that stop the hoist of
   (float)offset (e.g. the conversions in a different order per branch, `frame -= frames[i - 1]`), then copy the
   result to the other three.
2. QUEUE.md section 4 functions whose types and globals are already in headers (check with grep before starting);
   new code puts types/globals in headers from the start.
3. When a matched file is touched anyway, merge its remaining view (list above) and re-run `rule_check.py`.

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
Work through HANDOFF 'Next steps': the KeyBank::Sample family first (0x101af4c0 and its three twins), then
`decomp/QUEUE.md` section 4 functions whose types and globals are already in headers; put every new type and global in a shared header
so new matches count under the rules, and merge a file's remaining struct views (HANDOFF 'Deferred / open') when you
touch it. After each batch run `python tools/build_all.py --report ../progress/jp/report.json` and
`python tools/check_matches.py` from `decomp/`; commit and push only with no real loss. Keep everything
human-readable (named constants, liberal comments, informative names). Work autonomously. Rewrite HANDOFF.md at the
end with the Fully linked / Matched / Matched under the rules / Fuzzy numbers, keeping its Standing instructions
section and a continuation prompt for the next task, then end your turn."
