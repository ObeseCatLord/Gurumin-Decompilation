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
| Fully linked | 0.00% of code (147 of 252 units complete) |
| Matched | 11.84% of code (203750 of 1720206 bytes), 794 of 1212 functions |
| Matched under the rules | 6.47% of code (111245 bytes), 493 of the 794 matched functions |
| Fuzzy match | 15.13% |

"Matched under the rules": a matched function counts only when its file declares no struct/class/union and no
`extern` global of its own (`tools/rule_check.py`; build_all prints a `RULES` line). `python tools/rule_check.py`
lists the files still breaking rule 2 (301 matched functions), most matches first.

## State
- This session (1e0464f, 1ff1ea1, a91c234):
  - **Number_Draw / Number_DrawRightAligned / Number_DrawBack 0x10012990/0x10012d90/0x100131b0 matched** (new
    hud_digits.cpp + hud_digits.h, rule-clean): HUD numbers from the texWaku01 digit sheet.
  - **BoxCylinder_Overlap 0x10192200 matched** (box_overlap.cpp, rule-clean).
  - BoxSphere_Overlap 0x10192520 (4 bytes off) and CylinderSphere_Overlap 0x10192c90 (25 bytes off) drafted in
    box_overlap.cpp: both only miss the load order of `sphere->y + sphereOffset->y` at some sites (open.tsv, NOTES).
  - KeyBank::Sample 0x101af4c0: the spline blend factor written inside each Spline call removed the (float)offset
    hoist; still open: the linear flag `(keyFlags >> 2) & 1` is folded into `test al,4` (ten spellings tried),
    `i - 2` comes out as `(i - 1) - 1` in the original, and Spline is called as a thiscall member. The three
    KeyTrack twins (0x101afe00, 0x101af960, 0x101b0240) wait for it.
  - NOTES: x87 compare forms (`||` of `>=`/`<=`, `!(d >= r)`), hoisted branch floats, statement order before calls.
- Deferred / open (what keeps the other 301 matches pending): unchanged:
  - **Struct views in several files**: CollisionWorld (11 files), LastPosition (10), TextRenderer (5), Engine (4),
    WorldRenderer (4; WorldRenderer::Bind takes ModelRecord = AnimModel), MapObject (3), MmvPlayer (3), TriangleBatch
    (3), MapEntry, ModelRecord (= AnimModel), WorldNode, Object, Channel in node_world.cpp.
  - **Globals read as different types**: DAT_10255f08 (22 files), DAT_10802bf8 (15), DAT_10255160 (8), DAT_1021bdf8
    (7), DAT_103150c0 (7), DAT_10836c70 (6), DAT_10f6550c/DAT_10f65510, DAT_1101d884 and about 70 more
    (`python tools/globals_driver.py plan`).
  - GameFile 0x28 vs MovieFile 0x24; npc_greet.cpp skipped; ListBank::ReportError inline; DAT_10874848/DAT_104b7200
    both Game; ParamVector may be Vec4.
- Section 4 candidates looked at and left: 0x100d2110 (needs DAT_1021bdf8/DAT_10255160 settled first), 0x10164c10
  (unnamed lighting class), 0x1001bb20 (EH frame + draw-parameter struct), 0x10049b00 and 0x10026730 (float-heavy
  route code), 0x100360f0 (script tokenizer), 0x1004f8b7 (starts mid-code, no padding: probably part of a /Od unit),
  0x101a24e0 / 0x101a8780 / 0x101aa080 families (large EH-frame collision code with a dozen Vec4/Matrix4 temps).
- Look-alike families in sections 4-5 (same instruction sequence, >85%): 101aa080/101aaa20/101a9670,
  101713f0/10170580/10187520, 10167440/10167d90, 1017b210/1017b9b0, 10199e20/1019a6b0, 101a8780/101a8e60,
  1009a690/10099b30, 100fda40/100fc4a0. One match in a family usually gives the others.

## Next steps
1. Smaller section 4 functions whose neighbours are already matched in a rule-clean file (the way box_overlap.cpp
   and hud_digits.cpp went): pick by address next to matched code (`grep` symbols.tsv for the neighbours), decode
   the stack frame by hand from `asm.py`, write it, then fix the few scheduling diffs with variants.
2. A family from the list above (start with the smallest member: 1017b210, 1944 bytes).
3. The y-sum operand-order wall (0x10192520, 0x10192c90) and the KeyBank::Sample flag fold, if a new idea comes up.
4. When a matched file is touched anyway, merge its remaining view (list above) and re-run `rule_check.py`.

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
Work through HANDOFF 'Next steps': `decomp/QUEUE.md` section 4 functions next to already-matched rule-clean code
(decode the frame from `asm.py`, write, fix scheduling with variants), then one look-alike family from the list in
'State'; put every new type and global in a shared header so new matches count under the rules, and merge a file's remaining struct views (HANDOFF 'Deferred / open') when you
touch it. After each batch run `python tools/build_all.py --report ../progress/jp/report.json` and
`python tools/check_matches.py` from `decomp/`; commit and push only with no real loss. Keep everything
human-readable (named constants, liberal comments, informative names). Work autonomously. Rewrite HANDOFF.md at the
end with the Fully linked / Matched / Matched under the rules / Fuzzy numbers, keeping its Standing instructions
section and a continuation prompt for the next task, then end your turn."
