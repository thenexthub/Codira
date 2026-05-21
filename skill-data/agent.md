---
name: codira-agent
description: Agent workflow for making focused Codira changes with CLI feedback.
---

# Codira Agent Workflow

Use this when editing Codira code, examples, tests, docs, or a package. Codira is designed for explicit, machine-readable feedback; prefer the CLI JSON surfaces over guessing from prose.

## Start

Use the same compiler binary that will run the project:

```sh
codira --version
codira skills list
codira skills get codira-language
codira skills get codira-diagnostics
```

Inside the Codira repository checkout, prefer `bin/codira` over a global `codira`. For installed user projects, use the `codira` on `PATH` unless the user points at another binary.

## Edit Loop

1. Read the nearest `.0`, `codira.json`, tests, and examples before editing.
2. Make the smallest source change that satisfies the request.
3. Run a focused JSON check:

```sh
codira check --json <file-or-package>
```

4. When the compiler reports a diagnostic, inspect structured fields first:

```sh
codira explain <diagnostic-code>
codira fix --plan --json <file-or-package>
```

5. If behavior changes, add or update a `test` block or conformance fixture.
6. Validate with the narrowest command that covers the changed surface.

## Agent Rules

- Treat effects as capabilities, not ambient globals. Use `World`, `std.fs`, `std.args`, `std.env`, and similar APIs only where the target supports them.
- Keep examples copyable and runnable from the repository or package root.
- Prefer explicit types at public boundaries and when inference is unclear.
- Use `Maybe<T>`, explicit `raises`, and `check` instead of hidden failure.
- Do not invent syntax. Load `codira-language` when unsure.
- Do not invent CLI fields. Run the command with `--json` and read the data.

## Useful Focused Commands

```sh
codira check --json <input>
codira graph --json <input>
codira test --json <input>
codira size --json <input>
codira doctor --json
```

For CLI behavior or JSON contracts in the Codira repo, use the repository scripts listed by `AGENTS.md` or the project documentation.
