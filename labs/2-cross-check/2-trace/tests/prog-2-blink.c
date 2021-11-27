// rewritten 2-blink.c from 1-gpio
#include "rpi.h"
#include "trace.h"

void nop(void);

// countdown 'ticks' cycles
static inline void delay(unsigned ticks) {
    while(ticks-- > 0)
        nop();
}

void notmain(void) {
    enum { led1 = 20, led2 = 21 };

    uart_init();
    trace_start(0);

    gpio_set_output(led1);
    gpio_set_output(led2);

    for(int i = 0; i < 10; i++) {
        gpio_set_on(led1);
        gpio_set_off(led2);
        delay(1000000);
        gpio_set_off(led1);
        gpio_set_on(led2);
        delay(1000000);
    }
    trace_stop();
    clean_reboot();
}
