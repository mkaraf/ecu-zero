#ifndef ISOTP_H
#define ISOTP_H

#include <stdint.h>

typedef enum {
    ISOTP_PCI_SINGLE_FRAME = 0x0U,
    ISOTP_PCI_FIRST_FRAME  = 0x1U,
    ISOTP_PCI_CONSEC_FRAME = 0x2U,
    ISOTP_PCI_FLOW_CONTROL = 0x3U,
    ISOTP_PCI_INVALID      = 0xFU
} isotp_pci_type_t;

/* Classify the PCI type carried in the high nibble of the first CAN data byte. */
isotp_pci_type_t isotp_pci_type(uint8_t first_byte);

typedef enum {
    ISOTP_RX_OK,        /* SF payload delivered via on_indication */
    ISOTP_RX_IGNORED,   /* frame type not handled yet (FF/CF/FC) */
    ISOTP_RX_MALFORMED, /* frame is malformed according to ISO-TP rules */
    ISOTP_RX_OVERFLOW   /* frame payload exceeds the buffer size */
} isotp_rx_status_t;

typedef struct {
    uint8_t  *rx_buf;
    uint16_t  rx_buf_len;
    void    (*on_indication)(const uint8_t *data, uint16_t len, void *user_data);
    void     *user_data;
} isotp_ctx_t;

void isotp_init(isotp_ctx_t *ctx,
                 uint8_t *rx_buf, uint16_t rx_buf_len,
                 void (*on_indication)(const uint8_t *data, uint16_t len, void *user_data),
                 void *user_data);

isotp_rx_status_t isotp_on_frame(isotp_ctx_t *ctx,
                                  const uint8_t *data, uint8_t len,
                                  uint32_t now_ms);

#endif /* ISOTP_H */
