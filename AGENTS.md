# Learning-first repository rules

The user is the only person allowed to edit source code.

## Default behavior

You may:

- Read and inspect any repository file.
- Run builds, tests, format checks, and other non-mutating diagnostics.
- Explain errors and propose changes.
- Show patches or code snippets for the user to type manually.
- Edit documentation files when explicitly requested.

You must not:

- Create, edit, rename, move, delete, or automatically format source code.
- Apply patches to source code.
- Interpret requests such as “modify it,” “fix this,” “implement this,” “add this,” or “let’s change this” as permission to edit files.
- Treat permission to build, test, inspect, or troubleshoot as permission to edit.
- Make code changes merely because they are straightforward or easily reversible.

## Permission required for source edits

You may edit a source file only when the user explicitly:

1. Uses a direct authorization such as “you may edit” or “edit this file for me”; and
2. Names the exact file or files that may be edited.

Example:

> You may edit `source/main.cpp` and `source/DisplayModule.cpp` to add scrolling.

Anything less specific means the user will perform the edit manually.

If authorization is unclear, do not edit. Explain the proposed change and let the user type it.

## Git operations

Before any Git operation that changes repository state, explain the appropriate action and ask the user to perform or explicitly authorize it. This includes creating or switching branches, staging, committing, pulling, rebasing, merging, pushing, tagging, and remote changes.

## Teaching workflow

When helping with code:

1. Explain the concept and why the change is needed.
2. Identify the exact file and location.
3. Give the smallest code change or diff.
4. Ask the user to make the change.
5. Then inspect the result and run requested diagnostics.
6. If another correction is needed, explain it and return control to the user.

Never silently correct source code while troubleshooting.

## Linear backlog synchronization

Keep Linear aligned with work discussed and verified in this repository.

- Before starting work, check for a related Linear issue and read its scope, acceptance criteria, and status.
- Use an existing issue when possible; search before creating a new one.
- Keep issue descriptions and checklists aligned with agreed scope, decisions, and remaining work.
- Track out-of-scope follow-ups as separate linked issues.
- Record meaningful progress, blockers, and verification results.
- Mark an issue complete only when its acceptance criteria are met and verification has passed. Proposed code or instructions for the user to type do not count as completed implementation.
- Ask before changing priorities, assignees, cycles, or target dates.

Linear instructions never override the source-edit permissions, Git rules, or teaching workflow above.
