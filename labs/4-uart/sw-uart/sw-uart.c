#include "rpi.h"
#include "sw-uart.h"
#include "cycle-count.h"
#include "cycle-util.h"

#define CYCLES_PER_BIT 6076

// helper: cleans up the code.
static inline void timed_write(int pin, int v, unsigned usec) {
    gpio_write(pin,v);
    delay_us(usec);
}

void sw_uart_init_real(sw_uart_t *uart) {
    gpio_set_function(uart->tx, GPIO_FUNC_OUTPUT);
    gpio_set_function(uart->rx, GPIO_FUNC_INPUT);
    cycle_cnt_init();
}

// do this first: used timed_write to cleanup.
//  recall: time to write each bit (0 or 1) is in <uart->usec_per_bit>
void sw_uart_putc(sw_uart_t *uart, unsigned char c) {
    write_cyc_until(20, 0, cycle_cnt_read(), CYCLES_PER_BIT);
    for (int i = 0; i < 8; i++)
        write_cyc_until(20, (c & (1U << i)) >> i, cycle_cnt_read(), CYCLES_PER_BIT);
    write_cyc_until(20, 1, cycle_cnt_read(), CYCLES_PER_BIT);
}

void sw_uart_putk(sw_uart_t *uart, const char *msg) {
    for(; *msg; msg++)
        sw_uart_putc(uart, *msg);
}

// do this second: you can type in pi-cat to send stuff.
//      EASY BUG: if you are reading input, but you do not get here in 
//      time it will disappear.
int sw_uart_getc(sw_uart_t *uart, int timeout_usec) {
    unimplemented();
}
