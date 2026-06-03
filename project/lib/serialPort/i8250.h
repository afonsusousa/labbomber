#ifndef SERIAL_PORT_DEFS_H
#define SERIAL_PORT_DEFS_H

// COM1 base address and IRQ 
#define COM1_ADDR 0x3F8
#define COM1_IRQ  4

// Serial port register offsets relative to COM1 base 
#define SERP_DATA 0x00
#define SERP_DLL  0x00
#define SERP_DLM  0x01
#define SERP_IER  0x01
#define SERP_FCR  0x02
#define SERP_IIR  0x02
#define SERP_LCR  0x03
#define SERP_MCR  0x04
#define SERP_LSR  0x05
#define SERP_MSR  0x06
#define SERP_SCR  0x07

// Line Control Register bits
#define LCR_WORD_LENGTH_5 0x00
#define LCR_WORD_LENGTH_6 0x01
#define LCR_WORD_LENGTH_7 0x02
#define LCR_WORD_LENGTH_8 0x03
#define LCR_STOP_2        0x04
#define LCR_PARITY_NONE   0x00
#define LCR_PARITY_ODD    0x08
#define LCR_PARITY_EVEN   0x18
#define LCR_DLAB          0x80

// Interrupt Enable Register bits
#define IER_ENREC_INT     0x01
#define IER_ENTRANS_INT   0x02
#define IER_ERLS_INT      0x04
#define IER_EMSC_INT      0x08

// FIFO Control Register bits
#define FCR_EN_1          0x07
#define FCR_RX_RESET      0x02
#define FCR_TX_RESET      0x04
#define FCR_DMA_MODE      0x08
#define FCR_FIFO_1_BYTE   0x00
#define FCR_FIFO_4_BYTE   0x40
#define FCR_FIFO_8_BYTE   0x80
#define FCR_FIFO_14_BYTE  0xC0

// UART base clock for baud rate divisor calculation
#define FIXED_FREQUENCY   115200

#endif /* SERIAL_PORT_DEFS_H */
