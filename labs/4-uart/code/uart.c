// implement:
//  void uart_init(void)
//
//  int uart_can_getc(void);
//  int uart_getc(void);
//
//  int uart_can_putc(void);
//  void uart_putc(unsigned c);
//
//  int uart_tx_is_empty(void) {
//
// see that hello world works.
//
//
#include "rpi.h"

// Broadcomm doc: "If the enable bits are clear you will have no access to a
//                 peripheral. You can not even read or write the registers!"
#define AUX_ENABLE_REG  0x20215004 // used to enable/disable uart

// mini uart I/O
#define AUX_MU_IO_REG   0x20215040 // used to read/write the uart FIFOs

// mini uart interrupt enable
#define AUX_MU_IER_REG  0x20215044 // used to enable/disable uart interrupts

// mini uart interrupt identify (also has FIFO enable/disable bits)
#define AUX_MU_IIR_REG  0x20215048 // used to enable/disable uart FIFOs

// mini uart line control
#define AUX_MU_LCR_REG  0x2021504C // used to control line data format (i.e. 7/8-bit mode)

// mini uart line status
#define AUX_MU_LSR_REG  0x20215054 // read to see status of transmit/receive FIFOs (e.g. idle, empty, ready)

// mini uart extra control
#define AUX_MU_CNTL_REG 0x20215060 // used to enable/disable transmitter/receiver

// mini uart extra status
#define AUX_MU_STAT_REG 0x20215064 // used to check if transmitter FIFO has space available

// mini uart baudrate
#define AUX_MU_BAUD     0x20215068 // used to set baudrate? why does engler say not to use in broadcomm annots?

static void disable_uart(void) {
    // clear bit 0 to disable uart
    PUT32(AUX_ENABLE_REG, GET32(AUX_ENABLE_REG) & 0xFFFFFFFE);   // 0xE = 0b1110
}

static void enable_uart(void) {
    // set bit 0 to enable uart
    PUT32(AUX_ENABLE_REG, GET32(AUX_ENABLE_REG) | 0x00000001);   // 0x1 = 0b0001
}

static void disable_uart_rx(void) {
    // clear bit 0 to disable uart rx
    PUT32(AUX_MU_CNTL_REG, GET32(AUX_MU_CNTL_REG) & 0xFFFFFFFE); // 0xE = 0b1110
}

static void disable_uart_tx(void) {
    // clear bit 1 to disable uart tx
    PUT32(AUX_MU_CNTL_REG, GET32(AUX_MU_CNTL_REG) & 0xFFFFFFFD); // 0xD = 0b1101
}

static void enable_uart_rx(void) {
    // set bit 0 to enable uart rx
    PUT32(AUX_MU_CNTL_REG, GET32(AUX_MU_CNTL_REG) | 0x00000001); // 0x1 = 0b0001
}

static void enable_uart_tx(void) {
    // set bit 1 to enable uart tx
    PUT32(AUX_MU_CNTL_REG, GET32(AUX_MU_CNTL_REG) | 0x00000002); // 0x2 = 0b0010
}

static void disable_uart_interrupts(void) {
    // clear bit 0 to disable transmit interrupts
    PUT32(AUX_MU_IER_REG, GET32(AUX_MU_IER_REG) & 0xFFFFFFFE); // 0xE = 0b1110
    // clear bit 1 to disable receive interrupts
    PUT32(AUX_MU_IER_REG, GET32(AUX_MU_IER_REG) & 0xFFFFFFFD); // 0xD = 0b1101
}

static void clear_uart_rx_fifo(void) {
    // set bit 1 to clear the receive FIFO
    PUT32(AUX_MU_IIR_REG, GET32(AUX_MU_IER_REG) | 0x00000002); // 0x2 = 0b0010
}

static void clear_uart_tx_fifo(void) {
    // set bit 2 to clear the transmit FIFO
    PUT32(AUX_MU_IIR_REG, GET32(AUX_MU_IER_REG) | 0x00000004); // 0x4 = 0b0100
}

static void set_uart_8bit_mode(void) {
    // set bits 0-1 to enable 8-bit mode
    PUT32(AUX_MU_LCR_REG, GET32(AUX_MU_LCR_REG) | 0x00000003); // 0x3 = 0b0011
}

static void set_uart_baudrate(void) {
    // set bits 0-15 to configure baudrate
    PUT32(AUX_MU_BAUD, 115200);
}

// called first to setup uart to 8n1 115200  baud,
// no interrupts.
//  - you will need memory barriers, use <dev_barrier()>
//
//  later: should add an init that takes a baud rate.
void uart_init(void) {
    /*
     * Steps (I think):
     * 1. disable uart
     * 2. setup tx/rx gpio pins (Broadcomm: "GPIO pins should be set up first the before enabling the UART")
     * 3. enable uart (so we can touch uart regs)
     * 4. disable uart tx/rx (so we don't send garbage)
     * 5. disable uart interrupts
     * 6. clear tx/rx FIFO queues
     * 7. configure LCR for 8-bit mode
     * 8. configure baudrate
     * 9. enable uart tx/rx
     */

    // Engler: "You'll want to explicitly disable the mini-UART at the beginning
    //          to ensure it works if uart_init(void) is called multiple times
    //          (as can happen when we use a bootloader, or reboot())."
    disable_uart();

    // Broadcomm doc: "GPIO pins should be set up first the before enabling the UART"
    // TODO: setup tx/rx gpio pins
    // Uhh, is this right? What does 'pull low' mean?
    // gpio_set_function(14, GPIO_FUNC_ALT0);
    // gpio_set_function(15, GPIO_FUNC_ALT0);

    enable_uart();

    disable_uart_tx();
    disable_uart_rx();

    // Engler: " Disable interrupts"
    disable_uart_interrupts();

    clear_uart_tx_fifo();
    clear_uart_rx_fifo();

    set_uart_8bit_mode();

    // FIXME: This triggers BOOT_SUCCESS mismatch atm :-(
    // set_uart_baudrate();

    // Engler: "you will often want to fully enable the mini-UART's ability to
    //          transmit and receive as the very last step after configuring it.
    //          Otherwise it will be on in whatever initial state it booted up
    //          in, possibly interacting with the world before you can specify
    //          how it should do so."
    // TODO: Should we be enabling both tx and rx here?
    enable_uart_tx();
    enable_uart_rx();
}

// 1 = at least one byte on rx queue, 0 otherwise
static int uart_can_getc(void) {
    return 0;
}

// returns one byte from the rx queue, if needed
// blocks until there is one.
int uart_getc(void) {
	return 0;
}

// 1 = space to put at least one byte, 0 otherwise.
int uart_can_putc(void) {
    return 0;
}

// put one byte on the tx qqueue, if needed, blocks
// until TX has space.
void uart_putc(unsigned c) {
    PUT32(AUX_MU_IO_REG, c);
}

// simple wrapper routines useful later.

// a maybe-more intuitive name for clients.
int uart_has_data(void) {
    return uart_can_getc();
}


// return -1 if no data, otherwise the byte.
int uart_getc_async(void) { 
    if(!uart_has_data())
        return -1;
    return uart_getc();
}

// 1 = tx queue empty, 0 = not empty.
int uart_tx_is_empty(void) {
    unimplemented();
}

void uart_flush_tx(void) {
    while(!uart_tx_is_empty())
        ;
}
