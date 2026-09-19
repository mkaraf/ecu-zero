#include "isotp.h"

#include <string.h>
#include <assert.h>
#include <stddef.h>

static inline isotp_rx_status_t isotp_on_single_frame(isotp_ctx_t *ctx,
                                  const uint8_t *data, uint8_t len)
{
    isotp_rx_status_t status = ISOTP_RX_MALFORMED;

    uint8_t const payload_len = (uint8_t)(data[0U] & 0xFU); // payload length is in the low nibble of the first byte

    if((payload_len > 0U) && (payload_len <= 7U) && (len == (uint8_t)(payload_len + 1U)))
    {
        if(payload_len <= ctx->rx_buf_len)
        {
            memcpy(ctx->rx_buf, &data[1U], payload_len);
            ctx->on_indication(ctx->rx_buf, payload_len, ctx->user_data);
            status = ISOTP_RX_OK;
        }
        else
        {
            status = ISOTP_RX_OVERFLOW; // payload length exceeds buffer size
        }
    }

    return status;
}

static inline isotp_rx_status_t isotp_on_frame_router(isotp_ctx_t *ctx,
                                  const uint8_t *data, uint8_t len,
                                  isotp_pci_type_t const pci_type)
{
    isotp_rx_status_t status = ISOTP_RX_IGNORED;

    if(pci_type == ISOTP_PCI_SINGLE_FRAME)
    {
        status = isotp_on_single_frame(ctx, data, len);
    }
    else if(pci_type == ISOTP_PCI_FIRST_FRAME)
    {
        //status = isotp_on_first_frame(ctx, data, len, now_ms);
    }
    else if(pci_type == ISOTP_PCI_CONSEC_FRAME)
    {
        //status = isotp_on_consecutive_frame(ctx, data, len, now_ms);
    }
    else if(pci_type == ISOTP_PCI_FLOW_CONTROL)
    {
        //status = isotp_on_flow_control(ctx, data, len, now_ms);
    }
    else
    {
        // unknown PCI type, keep status as ISOTP_RX_IGNORED
    }

    return status;
}

isotp_pci_type_t isotp_pci_type(uint8_t first_byte)
{
    const uint8_t nibble = (uint8_t)(first_byte >> 4);
    isotp_pci_type_t pci_type = ISOTP_PCI_INVALID;

    if(nibble == 0x0U)
    {
        pci_type = ISOTP_PCI_SINGLE_FRAME;
    }
    else if(nibble == 0x1U)
    {
        pci_type= ISOTP_PCI_FIRST_FRAME;
    }
    else if(nibble == 0x2U)
    {
        pci_type = ISOTP_PCI_CONSEC_FRAME;
    }
    else if(nibble == 0x3U)
    {
        pci_type = ISOTP_PCI_FLOW_CONTROL;
    }
    else {
        pci_type = ISOTP_PCI_INVALID;
    }
    return pci_type;
}

void isotp_init(isotp_ctx_t *ctx,
                 uint8_t *rx_buf, uint16_t rx_buf_len,
                 void (*on_indication)(const uint8_t *data, uint16_t len, void *user_data),
                 void *user_data)
{
    assert(ctx != NULL);
    assert(rx_buf != NULL);
    assert(rx_buf_len > 0U);
    assert(on_indication != NULL);

    ctx->rx_buf = rx_buf;
    ctx->rx_buf_len = rx_buf_len;
    ctx->on_indication = on_indication;
    ctx->user_data = user_data;
}

isotp_rx_status_t isotp_on_frame(isotp_ctx_t *ctx,
                                  const uint8_t *data, uint8_t len,
                                  uint32_t now_ms)
{
    (void)now_ms; // unused for now
    isotp_rx_status_t status = ISOTP_RX_IGNORED;
    isotp_pci_type_t pci_type = ISOTP_PCI_INVALID;


    if((len <= 8U) && (len > 0U))
    {
        pci_type = isotp_pci_type(data[0U]);

        if( pci_type != ISOTP_PCI_INVALID)
        {
            status = isotp_on_frame_router(ctx, data, len, pci_type);
        }
        else
        {
            status = ISOTP_RX_MALFORMED;
        }
    }
    else
    {
        status = ISOTP_RX_MALFORMED;
    }

    return status;
}
