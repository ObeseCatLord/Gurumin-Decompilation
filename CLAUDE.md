# Claude Code instructions

Read `AGENTS.md` first: it holds the rules every agent follows here (typed members only, each type declared once,
no fakematches, readable commented source, no game files) and the build commands. Then read `NOTES.md` for the
confirmed findings and the compiler habits that decide matches.

- Use the decompilation skills from <https://github.com/lemiur/Game-Decompilation-Skill-For-Claude> (install steps
  in `AGENTS.md`).
- Work from `decomp/QUEUE.md`; record near-misses in `decomp/open.tsv`; log each batch in `decomp/BATCHES.md`.
- Keep each file's own line endings (see `AGENTS.md`).
- Long-term memory: if `willow` (willow-substrate) is installed, its hooks give recall across sessions. Durable
  project facts still belong in `NOTES.md` or `AGENTS.md`, where every collaborator can read them.
