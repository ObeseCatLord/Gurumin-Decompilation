# Claude Code instructions

Read `AGENTS.md` first: it holds the rules every agent follows here (typed members only, each type declared once,
no fakematches, readable commented source, no game files) and the build commands. Then read `NOTES.md` for the
confirmed findings and the compiler habits that decide matches.

- Use the decompilation skills from <https://github.com/lemiur/Game-Decompilation-Skill-For-Claude> (install steps
  in `AGENTS.md`).
- Work from `decomp/QUEUE.md`; record near-misses in `decomp/open.tsv`; log each batch in `decomp/BATCHES.md`.
- Keep each file's own line endings (see `AGENTS.md`).
- Start from `decomp/HANDOFF.md` and rewrite it before the session ends (state, next steps, continuation prompt).
- Plan in plan mode; delegate implementation to Sonnet subagents with self-contained briefs; verify their results.
- Use `trail skeleton/callers/ask` (if installed) instead of reading whole files.
