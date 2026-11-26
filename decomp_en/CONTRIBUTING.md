# Helping with the EN (Steam) decompilation

The English Steam release (`game_en_steam.exe`) is matched function by function with the compiler it was built with,
like the JP project in `decomp/`. Most EN functions have a JP twin, and many of those twins are already matched, so
the usual job is **porting matched JP source to EN** and fixing what the newer compiler does differently.

Read [`../AGENTS.md`](../AGENTS.md) first for the rules every change follows. Then read [`NOTES.md`](NOTES.md) for
the EN compiler habits.

## Where to start

New here? Take your first functions from section 2 of [`QUEUE.md`](QUEUE.md), **"JP twin matched"**.
- Each of these already has byte-matching JP source. `tools/port_jp.py` hands it to you with the EN disassembly next
  to it, so the work is mostly adjusting it for the newer compiler.
- Start with the smallest ones, at the top of the section. They teach the EN compiler's habits quickly.
- Move on to section 3 (JP twin not matched yet) or section 4 (EN-only) once a few have matched. Those start from
  Ghidra's output instead of finished source.

Pull requests written with an AI agent are welcome; see [With an AI agent](#with-an-ai-agent).

## How it works

| File | What it is |
|---|---|
| `jp_en_map.tsv` | One row per EN function: its JP twin (module, address, name), the twin's status, and a confidence (`high`/`medium`/`low`/`none`). Made by `tools/map_jp_to_en.py` from shared strings, constants, call sequences and call-graph neighbours. `none` means EN-only (launcher, Steam, unidentified). |
| `classify.tsv` | Game code vs library code (MFC, ATL, CRT). Only game code is counted and queued. |
| `QUEUE.md` | The work list, made by `tools/gen_queue.py`: walls worth a retry, then functions whose JP twin is matched, then mapped-but-unmatched twins, then EN-only functions, smallest first. `gen_queue.py` rebuilds `jp_en_map.tsv` first (about 12 s), so JP functions matched since the last run move into section 2. |
| `open.tsv` | Near-misses: what was tried and what still differs. Read a function's row before starting on it. |
| `obj_symbols.tsv` | Mangled C++ name → EN address, so the build knows which original function each compiled symbol is. |
| `../progress/en/report.json` | The progress report (ByteMatch reads it from CI). |

For one function, `python tools/port_jp.py <EN address>` prints:
- the map row;
- the matched JP source of the twin, cut from `decomp/src`;
- the EN disassembly;
- the EN callees with their JP names.

`--brief` prints the same as a ready-made brief for an AI agent.

## One-time setup (Windows)

1. **The exe.** Copy `game_en_steam.exe` from your Steam install into `re/bin/`. Check that it is the same build:
   its SHA-256 should be `4a27c727160d2565f02488883e86a34ac6e0f650aee528f0a1e24017eedc5e9e`. Never commit it;
   `re/bin/` is git-ignored.
2. **The compiler.** You need Visual Studio **Community 2013 with Update 4**, with "Microsoft Foundation Classes
   for C++" selected; the launcher code needs MFC. It's on [my.visualstudio.com](https://my.visualstudio.com/Downloads?q=Visual%20Studio%20Community%202013%20with%20Update%204),
   and a free Dev Essentials account is enough.
   - Check that `VC\bin\cl.exe` is **18.00.31101**.
   - Don't install Update 5: its compiler (18.00.40629) doesn't match the exe.
   - Set `GURUMIN_VC12` to your "Microsoft Visual Studio 12.0" folder, for example
     `setx GURUMIN_VC12 "C:\Program Files (x86)\Microsoft Visual Studio 12.0"`.
3. **Python 3** with `pip install capstone`. If `python` on your PATH isn't the one you want, set `GURUMIN_PY` to
   its `python.exe`.
4. **Ghidra export.** Install [Ghidra](https://github.com/NationalSecurityAgency/ghidra/releases) 12.x and set
   `GHIDRA` to its folder. Then run `re\run_ghidra_en.bat` once; it takes a while. It writes `re/export/game_en_steam.exe/`,
   which the tools read for function lists and Ghidra's decompilation. The export is derived from the game, so it is
   git-ignored too.
5. **Check.** From `decomp_en/`, run `python tools/build_all.py --report ../progress/en/report.json`. The `PROGRESS`
   line should show the same matched count as the committed report.

`run_cl.py` works around VS2013's compiler crashing when Windows uses UTF-8 as its ANSI code page; `build.bat`
handles that for you.

## Working on a function

1. **Pick.** Take a function from `QUEUE.md`, preferably from section 2 ("JP twin matched"); those are the quickest.
   Check [open pull requests](https://github.com/Tenome/Gurumin-Decompilation/pulls) that nobody else has it. Then
   **open a draft pull request early** titled `EN: <address> <name>`, so others, including the project's own agents,
   skip it.
2. **Read.** Run `python tools/port_jp.py <address>`, plus its `open.tsv` row if it has one.
3. **Write.** Put the source in `src/<topic>.cpp`, next to related functions if a file exists.
   - Carry over the JP names and comments.
   - Put types in `include/` headers: one declaration per type, typed members and never fixed offsets.
   - Add an `obj_symbols.tsv` row for each new mangled name.
4. **Build and compare.** Run `build.bat <file>.cpp`. It compiles the file with the EN flags (`/O2 /Oy- /GS- /EHsc`)
   and byte-compares every function in it. `python tools/sbs.py` shows ours and the original side by side.
5. **Know what usually differs from JP.**
   - VS2013 inlines more (`/Ob2`): a JP call may be inlined in EN.
   - Floats use SSE2.
   - EN sometimes adds fields or changes behaviour. Then the JP source is only a guide.
   - Low-confidence map rows can be wrong twins: check before porting.
6. **Run the gate** before a pull request:
   ```
   python tools/build_all.py --report ../progress/en/report.json
   python tools/check_matches.py
   python tools/gen_queue.py
   ```
   `check_matches.py` must report no lost match. A renamed function shows as LOST + new at the same address; update
   its `obj_symbols.tsv` row.
7. **Record.** If it doesn't match after a reasonable try, add an `open.tsv` row saying what you tried and what
   still differs. A good near-miss note is a useful contribution too.
8. **Pull request.** Include the source, header, `obj_symbols.tsv`, `open.tsv`, `progress/en/report.json` and
   `progress/en/progress.svg`. Paste the `PROGRESS` and `check_matches` lines into the description. Keep each
   file's own line endings (see `AGENTS.md`).
   - Generated files (`jp_en_map.tsv`, `QUEUE.md`, the report) may conflict with other work. That's fine: the
     maintainer rebuilds them when merging.
   - Each pull request is checked before it is merged: the build is rerun on top of the latest `master`, every
     claimed match is confirmed, no existing match may be lost, and the rules below must hold.

## With an AI agent

Point Claude Code (or another agent) at the repository. Tell it to read `AGENTS.md`, `decomp_en/CONTRIBUTING.md`
and `decomp_en/HANDOFF.md`, then give it one function:
"match EN `<address>` following decomp_en/CONTRIBUTING.md". `python tools/port_jp.py <address> --brief` produces a
self-contained brief that a cheaper model can work from. The project's own loop works that way: Opus picks and
reviews, and Sonnet subagents port.

## Rules that decide whether a match counts

A function is not matched if it:
- uses fixed offsets instead of typed members;
- declares a type more than once;
- is a fakematch: code written only to coax the compiler, which no programmer would write.

Never commit game files: no exe, no Ghidra export, no extracted assets.
