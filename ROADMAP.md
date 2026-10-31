# ECU Zero — Roadmap

Hardware bring-up done: CubeMX project in `firmware/`, blinky verified on
the board.

## ISO-TP slices

- [ ] SF RX + TX
- [ ] Segmented RX (BS=0, no timers)
- [ ] Segmented TX consuming one FC
- [ ] BS and STmin on TX
- [ ] Timers, one at a time, starting with N_Cr
- [ ] Abort paths
