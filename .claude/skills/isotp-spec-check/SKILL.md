---
name: isotp-spec-check
description: Step 2 of an ECU Zero slice. Check a slice brief and docs/requirements/isotp.md against ISO 15765-2 and list ambiguities, gaps and conflicts before any header or test is written.
---

# ISO-TP spec check

Run this at step 2 of a slice, after Michal's slice brief and before any
interface proposal. Output is a report in chat. Do not write code, tests,
headers, or edit `docs/requirements/isotp.md`.

## Inputs
- The slice brief (requirement IDs, acceptance criteria, out of scope).
- `docs/requirements/isotp.md`.
- Current public headers in `src/include/`.
- Existing tests in `tests/` (to spot conflicts with what is already pinned).

## Procedure
1. List the requirement IDs in the brief. Read each one.
2. Walk the trap checklist below. For each item that touches this slice,
   check the requirements cover it unambiguously.
3. Check the requirements against the current headers and tests. A test
   that contradicts a requirement is a finding, not something to fix.
4. Check each requirement line marked `[VERIFY]` or `[DECIDE]` that the
   slice depends on.
5. Write the report.

## Trap checklist
Frame level
- DLC 0, DLC shorter than PCI requires, DLC longer (padding) on RX.
- Padding byte and DLC on TX.
- Reserved PCI nibbles 4–F.

SF
- SF_DL 0 and SF_DL > 7 on classic CAN.
- SF larger than the RX buffer.
- SF arriving mid-reception.

FF / segmented RX
- FF_DL below the minimum for the addressing mode.
- FF_DL escape (FF_DL = 0, 32-bit length) and escaped values ≤ 4095.
- FF_DL larger than the buffer → FC OVFLW.
- FF on the functional address.
- FF arriving mid-reception.
- Extra bytes in the last CF beyond FF_DL.

CF
- SN starts at 1, wraps 0xF → 0x0.
- Wrong SN, duplicate SN.
- CF with no reception in progress.
- BS counting and when the next FC is due.

FC / segmented TX
- FS CTS / WAIT / OVFLW / reserved.
- BS = 0 vs BS > 0.
- STmin ms range, µs range 0xF1–0xF9, reserved values → 0x7F.
- STmin granularity vs the ms time base.
- N_WFTmax and consecutive WAITs.
- FC arriving when not waiting for one.

Timers
- Which timer starts and stops on which event (N_As, N_Ar, N_Bs, N_Br, N_Cs, N_Cr).
- N_Br and N_Cs are performance constraints, not error timeouts.
- uint32 ms wrap-around.
- What "now" is when a frame and a timeout expire in the same call.

Concurrency and API
- RX and TX in progress at the same time.
- TX request while TX is busy.
- Re-entrancy: callbacks calling back into the module.
- Lifetime of indicated data.
- How errors (N_Result codes) reach the caller.

## Output format
One finding per line, grouped:

**Interface-shaping** (stop: Michal decides before step 3)
- `<req ID or "missing">` — the question — options — your recommendation.

**Local** (become tests marked `/* ASSUMPTION: ... */` in step 4)
- `<req ID>` — the assumption you propose.

**Conflicts** (requirement vs existing test or header)
- `<req ID>` vs `<file:line>` — what disagrees.

**Clause references to verify**
- `<req ID>` — ISO 15765-2 clause as you recall it.

End with: "Interface-shaping findings: N. Waiting for decisions." or
"No interface-shaping findings. Ready for step 3."

## Rules
- You have no copy of ISO 15765-2. Clause numbers and values are from
  general knowledge; say so, never present them as quotes.
- Never decide behaviour that is interface-shaping. Ask.
- Skip checklist items outside the slice scope. Do not report them.
- Keep the report short. No restating requirements that are already clear.
