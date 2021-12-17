#include "rpi.h"

static void my_putk(char *s) {
    for(; *s; s++)
        uart_putc(*s);
}

void notmain(void) {

    char byte;
    while ((byte = uart_getc()) != 'X')
        uart_putc(byte);

    // my_putk("fuck yeah this works\n");

    // hack to make sure aux is off.
    // dev_barrier();
    // Engler: "Make sure you read-modify-write --- don't kill the SPIm enables"
    // Also Engler: "PUT32(0x20215004, 0)"
    // PUT32(0x20215004, 0);
    // dev_barrier();

    // uart_init();
    // my_putk("hello world\n");
}
