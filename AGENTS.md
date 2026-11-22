# Gurumin matching decompilation: agent guide

This file is for AI coding agents (Claude Code and others) and for the humans who direct them. It overrides the
defaults of any skill. Read it, then `NOTES.md` (confirmed findings and compiler habits), before changing code.

The goal: reconstruct the source of `bin\APP.dll` (Gurumin, Japanese PC release, 2006) so the original compiler
emits the original bytes, function by function, in source a modder can read. Correct structure outranks any score.

## Hard rules (a function that breaks one is not matched)

1. **Use typed members, never fixed offsets.** Write `obj->radius`, `model->flags |= MODEL_DIRTY`, never
   `*(float *)((char *)obj + 0x58)` or `((int *)p)[7]`. If a field is unknown, add it to the struct with a
   placeholder name (`m_0058`) and a comment; retype the member until the cast disappears. Offset arithmetic on
   `this` and offset macros count as fixed offsets too. A function that breaks this rule is **not matched**, even
   with identical bytes.
2. **Declare each type once, and only once,** in a shared header under `decomp/include/`. No per-file copies, no
   per-file "views" of a struct with different padding. Each global is declared once, `extern` in a header.
3. **No fakematches.** No dummy locals, `volatile` carriers, steering blocks, permuter noise or inline assembly to
   move a score. If only an unreadable spelling matches, keep the readable one and record the function in
   `decomp/open.tsv` with the reason.
4. **Readable and commented.** A comment above every function saying what it does and who calls it, comments on
   fields, constants and magic numbers. Named locals, real field names, early returns, plain loops. Readability beats
   a match.
5. **No game files.** Never commit anything from the game (`re/bin/`, extracted assets, Ghidra exports, save
   files). Users supply their own copy.
6. **Claim "matched" only from the build's printed `MATCH` line,** never from memory or a ledger.

These are the `decomp-code-rules` skill's [match] rules R1, R2, R4 and R17, tightened for this project.

### Existing code and these rules

Code written before 2026-11-22 declares structs per file (`Vec4` in about 110 files, `CharRecord` in about 20,
`extern "C"` globals in every file). Those functions match byte for byte but break rule 2, so under these rules they
are **MATCH-PENDING** until their types move into shared headers. The progress report still counts bytes; it does not
yet check the rules. Consolidating the types is open work (see `decomp/BACKLOG.md`). New code follows the rules from
the start: put new types in a header, include it, and extend the existing header type instead of declaring a view.

## Skills

Use the decompilation skills by lemiur: <https://github.com/lemiur/Game-Decompilation-Skill-For-Claude>
(`matcher`, `wall-identifier`, `helper-recovery`, `permute`, `decomp-code-rules`, `orchestrator`). If they are not
installed, clone the repository and copy `.claude/skills/*` to `~/.claude/skills/` (all projects) or to this
repository's `.claude/skills/`. Claude Code finds them automatically.

- `matcher` for every function; `wall-identifier` when a function plateaus; `helper-recovery` when a body mixes
  domain code with another class's internals; `permute` only for a diagnosed register/schedule residue.
- `decomp-code-rules` to review a match before it counts.
- `orchestrator` to run parallel workers (one git worktree per worker, disjoint files).

## Project adapter

| verb | this project's command (run from `decomp/`) |
|---|---|
| `match <unit>` | `build.bat <file.cpp>` (from bash: `powershell -c "cmd /c '.\build.bat' file.cpp"`): compiles one file and byte-compares every function in it with APP.dll, relocations masked; prints `MATCH` or `DIFF ... diffs=N first@[k]` |
| `build` / `report` | `python tools/build_all.py --report ../progress/jp/report.json` (all files, report and progress meter) |
| `diff <function>` | `python tools/sbs.py build/<file>.obj <symbol> <address>`: our code and the original side by side |
| `disasm` / `decompile` | `python tools/asm.py <address> <length>`, `python tools/fn.py <address>` (Ghidra output; needs `re/export/`) |
| variants | `python tools/variants.py <file.cpp> <n>` builds each `#if V == k` block; `python tools/pick.py <file.cpp> <k>` keeps one (it does not understand `#if V != k`) |
| queue | `python tools/gen_queue.py` regenerates `decomp/QUEUE.md`; `python tools/todo.py` lists what is left |
| names | `python tools/sync_symbols.py` after editing `symbols.tsv` |

**Toolchain:** Visual Studio .NET 2002, cl 13.00.9466 (the exact build in APP.dll's Rich header), flags
`/O2 /Ob1 /Op /Gy /GS /GX`. `decomp/env.bat` has the author's install path; set `GURUMIN_VC` to your `Vc7` folder.
Experiment flags go in `GURUMIN_CFLAGS`. Later compilers (VS .NET 2003) schedule differently and will not match.

**Inputs you supply:** the retail JP `bin\APP.dll` copied to `re/bin/APP.dll` (never modify it), and a Ghidra export
(`re\run_ghidra_export.bat` → `re/export/APP.dll/`) for `fn.py`. Both are gitignored.

**Symbols:** functions are written as `FUN_<address>` (C linkage) or as named C++ methods. Every C++ symbol needs a
row in `decomp/obj_symbols.tsv` (`<mangled name><TAB><address>`) so the build can compare it; get the mangled name
from the compiled object. Give every function a readable name in `symbols.tsv`.

## Workflow for one batch

1. Pick work from `decomp/QUEUE.md` (walls to retry, then functions by size). Check `decomp/open.tsv` and `NOTES.md`.
2. Read the original with `asm.py` (Ghidra's output is a guide, often wrong about stack layout).
3. Write the function in the source file that holds its neighbours, with its types in headers.
4. Iterate with `build.bat` and `sbs.py` until `MATCH`. Patterns that worked before are in `NOTES.md`; add new ones.
5. If it will not match readably, record it in `decomp/open.tsv` (`<address><TAB>open<TAB><reason>`) and move on.
6. After the batch:
   - `python tools/build_all.py --report ../progress/jp/report.json`, then `python tools/sync_symbols.py`, then
     `python tools/gen_queue.py`;
   - add a row to `decomp/BATCHES.md`;
   - check `git diff --stat` for whole-file line-ending changes (below), commit and push.

## Line endings

Files mix CRLF and LF (`NOTES.md` and `decomp/BATCHES.md` are CRLF). Python text-mode rewrites, `sed -i` and
`tools/pick.py` can flip a whole file, which hides the real change in the history. Edit through binary reads and
writes that keep the file's own newline, and check `git diff --stat` before each commit.

## Working with others

- One worker per set of files. Parallel workers use `git worktree` on their own branches and never touch the same
  `.cpp` or header at the same time; merge through the build (every file still prints its old `MATCH` lines).
- Shared headers are shared state: when you extend a header type, rebuild every file that includes it
  (`build_all.py`) before committing.
- Commit messages name the change: `<Function> matched (<total> matched)`, `<Function> draft`, `tools: ...`.
- Record findings in `NOTES.md` (facts and compiler habits only; no diaries). Progress history lives in git and
  `decomp/BATCHES.md`.

## Layout

| Path | Contents |
|---|---|
| `decomp/src/` | reconstructed source, one file per area of the game |
| `decomp/include/` | shared headers (the one place each type is declared) |
| `decomp/tools/` | build, compare and inspection tools |
| `decomp/QUEUE.md`, `open.tsv`, `BATCHES.md`, `BACKLOG.md` | work queue, near-misses with reasons, batch log, larger open items |
| `symbols.tsv` | function names and evidence |
| `docs/` | file formats and modding notes |
| `tools/` | modding tools (file format converters) |
| `NOTES.md` | confirmed findings, compiler habits that decide matches |
