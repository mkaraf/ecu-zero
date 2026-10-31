# ISO-TP requirements (WP-A2)

Owner: Michal. Agents never edit this file.
One line per requirement. Test names carry the ID.
`[DECIDE]` = open, Michal must choose before the slice that needs it.
`[VERIFY]` = value from memory of ISO 15765-2, check against the standard.

Scope: classic CAN (DLC ≤ 8), normal addressing, 11-bit IDs, one channel.
Out of scope: CAN FD, extended/mixed addressing, 29-bit IDs, multiple channels.

## A2-R1 Frame handling (PCI)

- A2-R1.1 PCI type is the high nibble of byte 0: 0 SF, 1 FF, 2 CF, 3 FC.
- A2-R1.2 PCI nibble 4–F → frame ignored, reported as malformed.
- A2-R1.3 Frame with DLC 0 → ignored; byte 0 is not read.
- A2-R1.4 RX accepts frames longer than the PCI requires (padding). Bytes after the payload are ignored.
- A2-R1.6 Frame passed with length > 8 → ignored, malformed; no byte is read.
- A2-R1.5 TX pads every frame to DLC 8 with a padding byte configurable at init, default 0xCC.

## A2-R2 Single Frame

- A2-R2.1 SF_DL is the low nibble of byte 0; valid range 1–7.
- A2-R2.2 SF_DL = 0 or SF_DL > 7 → ignored, malformed.
- A2-R2.3 DLC < SF_DL + 1 → ignored, malformed.
- A2-R2.4 Valid SF → payload copied to the RX buffer and indicated once with length SF_DL.
- A2-R2.5 SF_DL > RX buffer size → ignored, nothing copied, nothing indicated.
- A2-R2.6 TX of a message of 1–7 bytes is sent as one SF.
- A2-R2.7 SF arriving during a segmented reception aborts it (error N_UNEXP_PDU) and is processed as a new message. [VERIFY]

## A2-R3 First Frame and segmented reception

- A2-R3.1 FF_DL is the 12 bits after the PCI nibble.
- A2-R3.2 FF_DL = 0 → escape: FF_DL is the next 4 bytes, big-endian, payload starts at byte 6.
- A2-R3.3 FF_DL < 8 (normal addressing) → ignored, malformed. Same for escaped FF_DL ≤ 4095. [VERIFY]
- A2-R3.4 FF_DL > RX buffer size → send FC OVFLW, no reception started.
- A2-R3.5 Valid FF → copy first payload bytes, send FC CTS, start N_Cr.
- A2-R3.6 FF arriving during a segmented reception aborts it (N_UNEXP_PDU) and starts a new one.
- A2-R3.7 FF received on the functional address → silently discarded, no FC sent.
- A2-R3.8 Reception completes when FF_DL bytes are received; indicated once with length FF_DL. Bytes beyond FF_DL in the last CF are ignored.

## A2-R4 Consecutive Frame and sequence numbers

- A2-R4.1 SN is the low nibble of byte 0. First CF after FF has SN 1.
- A2-R4.2 SN increments by 1 per CF and wraps 0xF → 0x0.
- A2-R4.3 Wrong SN → reception aborted, error N_WRONG_SN, nothing indicated.
- A2-R4.4 CF with no reception in progress → ignored.
- A2-R4.5 After every BS CFs (BS > 0) the receiver sends FC CTS again. BS = 0 → no further FC.

## A2-R5 Flow Control

- A2-R5.1 FS is the low nibble of byte 0: 0 CTS, 1 WAIT, 2 OVFLW. 3–F invalid.
- A2-R5.2 Byte 1 is BS, byte 2 is STmin.
- A2-R5.3 STmin 0x00–0x7F = 0–127 ms.
- A2-R5.4 STmin 0xF1–0xF9 = 100–900 µs.
- A2-R5.5 STmin reserved values (0x80–0xF0, 0xFA–0xFF) → treated as 0x7F.
- A2-R5.6 TX enforces STmin as a minimum gap between CFs. [DECIDE: time base is ms, so µs values round up to 1 ms — compliant because STmin is a minimum]
- A2-R5.7 TX on FC CTS: send BS CFs (BS = 0: all remaining), then wait for the next FC.
- A2-R5.8 TX on FC WAIT: restart N_Bs, keep waiting. TX does not count WAITs.
- A2-R5.9 TX on FC OVFLW → abort, N_BUFFER_OVFLW.
- A2-R5.10 TX on invalid FS → abort, N_INVALID_FS.
- A2-R5.11 FC when TX is not waiting for one → ignored.
- A2-R5.12 FC sent by the RX side uses configurable BS and STmin, defaults BS 0, STmin 0.
- A2-R5.13 RX side never sends FC WAIT, i.e. N_WFTmax = 0 and N_WFT_OVRN cannot occur. [DECIDE]

## A2-R6 Timers

All timeouts configurable at init. Defaults 1000 ms. [VERIFY]

- A2-R6.1 N_As: TX frame confirmation timeout → abort, N_TIMEOUT_A. [DECIDE: needs a TX-confirmation input from the driver]
- A2-R6.2 N_Ar: same as N_As for frames sent by the RX side (FC).
- A2-R6.3 N_Bs: TX waiting for FC → abort, N_TIMEOUT_Bs.
- A2-R6.4 N_Cr: RX waiting for the next CF → abort, N_TIMEOUT_Cr.
- A2-R6.5 N_Br and N_Cs are performance constraints only (time to send our FC / next CF), no timeout error. [VERIFY: (N_Br + N_Ar) < 0.9·N_Bs, (N_Cs + N_As) < 0.9·N_Cr]
- A2-R6.6 Time is a uint32 ms parameter; elapsed time uses unsigned subtraction so wrap-around after ~49.7 days is handled.

## A2-R7 Addressing

- A2-R7.1 CAN IDs (physical RX 0x7E0, TX 0x7E8, functional RX 0x7DF) are handled by the driver layer, not by the ISO-TP module. The driver filters IDs and sends with the TX ID.
- A2-R7.2 Each received frame is passed to the ISO-TP module with an addressing type: physical or functional.
- A2-R7.3 Functional addressing: only SF is accepted (FF per A2-R3.7; CF/FC ignored).

## A2-R8 Resources and API behaviour

- A2-R8.1 No dynamic allocation.
- A2-R8.2 RX buffer is caller-owned, passed at init; target size 512 bytes.
- A2-R8.3 No call blocks or busy-waits; each call does bounded work.
- A2-R8.4 Timers and TX pacing are driven by a periodic call taking `now_ms`. Deferred: name and shape decided in the segmented RX slice; not part of slice 1.
- A2-R8.5 Frames leave through a TX callback; the module never calls a driver.
- A2-R8.6 RX and TX are independent: one reception and one transmission can be in progress at the same time, with separate state.
- A2-R8.7 Reception outcome → one indication callback carrying an N_Result (mirrors N_USData.ind): N_OK with data and length, or an error code with no data.
- A2-R8.8 Transmission outcome → a separate confirmation callback carrying an N_Result (mirrors N_USData.con).
- A2-R8.9 New TX request while a TX is in progress → rejected with a busy status, current TX unaffected.
- A2-R8.10 Indication data stays valid only for the duration of the callback.
- A2-R8.11 All configuration (buffer, pad byte, callbacks, later BS/STmin/timeouts) is passed to init in one config struct.
- A2-R8.12 The indication carries the N_Result and the addressing type of the received message (mirrors N_TAtype); data is NULL and length 0 on failure.
- A2-R8.13 The driver reports a completed frame transmission through a separate input (e.g. `isotp_on_tx_done`). The confirmation callback fires from that input, never from inside the send call.
- A2-R8.14 The send call returns OK, BUSY (R8.9), INVALID_LEN (length 0) or TOO_LONG (> 7 bytes; slice 1 only, removed when segmented TX lands).
- A2-R8.15 The TX callback returns success/failure. On failure the send call returns an error, the TX returns to idle and no confirmation follows.
- A2-R8.16 TX data is caller-owned and must stay valid until the confirmation.
- A2-R8.17 Calling the send function from inside the indication or confirmation callback is allowed. Calling the frame-input function from inside any callback is not.
