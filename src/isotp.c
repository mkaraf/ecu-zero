#include "isotp.h"

isotp_pci_type_t isotp_pci_type(uint8_t first_byte)
{
    const uint8_t nibble = (uint8_t)(first_byte >> 4);

    if (nibble > 0x3U) {
        return ISOTP_PCI_INVALID;
    }
    return (isotp_pci_type_t)nibble;
}
