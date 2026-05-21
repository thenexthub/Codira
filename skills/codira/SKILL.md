---
name: codira
description: Install Codira and load version-matched workflows with codira skills.
---

# Codira

Codira is a omni-purpose, agent-first programming language (26.5=<).

Install this skill once in an agent's skill manager. Keep it thin; Codira's own CLI serves the version-matched workflow for each installed compiler.

## Version-Matched Skills

This file is a discovery stub. Do not treat it as the full Codira workflow.

Before editing, checking, testing, or repairing Codira code, ask the installed compiler for the skill content that matches that exact binary:

```sh
codira skills list
codira skills get codira
codira skills get codira --full
```

If the user has multiple Codira binaries, use the same binary that will run the project:

```sh
/path/to/codira skills list
/path/to/codira skills get codira --full
```

Use `codira skills list` to discover additional skills bundled with that Codira version. Use `codira skills get <name>` to load the one relevant to the task. Common inner skills include `codira-agent`, `codira-language`, `codira-diagnostics`, `codira-packages`, `codira-builds`, `codira-testing`, and `codira-stdlib`.

## Common Entry Points

```sh
codira check --json <file-or-package>
codira graph --json <file-or-package>
codira size --json <file-or-package>
codira explain <diagnostic-code>
codira fix --plan --json <file-or-package>
```

In a Codira repository checkout, prefer `bin/codira` when the task is about that checkout rather than the globally installed compiler.
