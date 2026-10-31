# ECU Zero — context for Claude Code

## What this is
A UDS (ISO 14229) diagnostic server over ISO-TP (ISO 15765-2), targeting
an STM32 Nucleo-F446RE. Protocol layers are developed and tested host-only.
Board bring-up is done: CubeMX project in `firmware/`, cross-compiled in WSL,
blinky verified on the board. ISO-TP on hardware is not started.

## Hard constraints
- C11. No dynamic allocation anywhere.
- `src/isotp.*` and future `src/uds.*` must contain NO HAL calls, no
  register access, no STM32 headers. They must compile and run on x86.
  HAL and STM32 code lives only under `firmware/`.
- Time enters as a parameter (`now_ms`). Transmission leaves via a
  function-pointer callback. Never call a clock or a driver directly.
- Buffer ownership is the caller's: the reassembly buffer is passed at
  init. FF_DL larger than the buffer is rejected with FC overflow.
- Builds with `-Wall -Wextra -Wconversion -Werror`. Warnings are errors.
- Unity test runners are hand-written. No Ruby generator.

## Build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure

## Sources of truth
- Requirements: `docs/requirements/isotp.md`, atomic IDs (e.g. `A2-R5.3`).
  Owned by Michal. Never edit it. If a requirement is ambiguous, missing,
  or looks wrong, stop and report — do not decide the behaviour yourself.
- Agent roles: `.claude/agents/*.md` (the only place roles are defined).
- Slice order and progress: `ROADMAP.md` (a checklist, nothing more).
- This file is the workflow contract. No separate process documents.

## Minimal paperwork — do not create docs nobody reads
- No devlog, status file, or assumptions file.
- Design rationale goes in doc comments in the public headers, next to
  the code it explains.
- A local assumption is a test with a one-line comment
  `/* ASSUMPTION: ... */`. The test is the record.
- History is the git log and PR descriptions, which Claude writes.

## How we work (AI-driven SDLC)
Michal owns the requirements, the public interfaces, test review, and the
merge. Claude does spec analysis, interface proposals, tests,
implementation, review drafts, and doc updates.

Work is done one slice at a time, in this order:
1. Slice brief (Michal): requirement IDs, acceptance criteria, out of scope.
2. Spec check: list ambiguities against ISO 15765-2, each classified as
   interface-shaping or local. Stop for Michal on interface-shaping ones.
   Local ones become tests marked as assumptions (see above).
3. Interface: propose public header changes only. Wait for approval.
4. Tests: written from the requirements and approved headers only, never
   from an implementation. Every test name carries its requirement ID.
   Wait for Michal's test review.
5. Implementation: make the approved tests pass. Never modify tests or
   public headers. If a test looks wrong, stop and report.
6. All CI gates green before asking for review.
7. Close-out: tick the slice in `ROADMAP.md` in the final commit.

Commit rule: one slice is one PR, made of separate commits in this order:
1. interface commit — public headers only;
2. tests commit — `tests/` only;
3. implementation commit(s) — `src/` only.
CI checks each commit's paths. Test and sanitizer gates run on the PR head,
so main stays green while the tests stay reviewable before any
implementation exists.

## Communication
- When I get something wrong, say so directly. Do not soften it.
- Outside an approved slice step, propose the approach in a sentence or
  two and wait for me to agree before writing code.
- Ask before editing requirements, `CLAUDE.md`, or `.claude/agents/`.
- I have no copy of ISO 15765-2 text. Spec checks give clause references
  from general knowledge for Michal to verify; they are never quotes.
