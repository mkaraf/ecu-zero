#ifndef ISOTP_H
#define ISOTP_H

#include <stdint.h>

typedef enum {
    ISOTP_PCI_SINGLE_FRAME = 0x0,
    ISOTP_PCI_FIRST_FRAME  = 0x1,
    ISOTP_PCI_CONSEC_FRAME = 0x2,
    ISOTP_PCI_FLOW_CONTROL = 0x3,
    ISOTP_PCI_INVALID      = 0xF
} isotp_pci_type_t;

/* Classify the PCI type carried in the high nibble of the first CAN data byte. */
isotp_pci_type_t isotp_pci_type(uint8_t first_byte);

#endif /* ISOTP_H */
