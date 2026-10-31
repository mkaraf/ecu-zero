# ECU Zero — Roadmap

**Goal:** build a CAN node with a working UDS diagnostic server, prove it
correct with an automated test suite, extend it with an on-device classifier,
and write the safety analysis that covers the result.

**Status:** planning complete, hardware on order, no code written yet.

---

## Milestones

### A — CAN node with UDS diagnostic server
The foundation. Everything else attaches to this.

- CAN driver bring-up on STM32 Nucleo-F446RE
- ISO-TP (ISO 15765-2) transport layer
- UDS server: session control, SecurityAccess, DTC read/clear, ResponsePending
- Host-side test client via python-can + udsoncan

**Done when:** a host script can open an extended session, unlock via
SecurityAccess, read and clear a DTC, and survive a long-running service that
requires ResponsePending.

### B — Test architecture and CI

- HAL abstraction so logic is testable off-target
- Unit tests with Unity + CMock
- GitHub Actions running build, tests, static analysis, coverage
- Hardware-in-the-loop tests driven by pytest against the real board
- Requirements traceability matrix: requirement → test → result

**Done when:** a green CI run means something, and any requirement can be traced
to the test that covers it.

### C — On-device traffic sign classifier

- Quantised CNN trained on GTSRB
- Deployed with TFLite Micro or hand-written CMSIS-NN kernels
- Measured: flash footprint, RAM, inference latency, accuracy after quantisation

Warm-up planned: a small neural network written from scratch in C, to
understand the inference path before handing it to a framework.

### D — Safety concept

- UDS service exposing classifier output over CAN
- Written safety concept: HARA, ASIL determination, safety goals
- SOTIF analysis per ISO/PAS 21448 for the perception component
- Argument for whether a statistical component can be trusted in this role, and
  under what conditions

---

## Hardware

| Item | Purpose |
|---|---|
| STM32 Nucleo-F446RE | Target MCU |
| SN65HVD230 transceivers | CAN physical layer |
| CANable 2.0 clone (slcan firmware) | Host CAN interface |
| Logic analyzer | Bus-level debugging, timing verification |

## Host toolchain

Windows. python-can over slcan, udsoncan, pytest. Unity and CMock for unit
tests. GitHub Actions for CI.

## Standards in scope

ISO 26262 (functional safety), ISO/PAS 21448 (SOTIF), ISO 15765-2 (ISO-TP),
ISO 14229 (UDS), EU Cyber Resilience Act (vulnerability handling, SBOM).
