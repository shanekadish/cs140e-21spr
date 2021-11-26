// trivial little fake r/pi system that will allow you to debug your
// gpio.c code.  can compile on your laptop and then run the code there
// too:
//      # compile using your normal compiler.
//      % gcc fake-pi.c gpio.c 1-blink.c
//      % ./a.out
//      calling pi code
//      GET32(20200008) = 643c9869
//      ...
//      PUT32(2020001c) = 100000
//      PUT32(20200028) = 100000
//      pi exited cleanly
//
// some nice things:
//  1. use a debugger;
//  2. by comparing the put/get values can check your code against other
//     people (lab 3)
//  3. you have memory protection, so null pointer writes get detected.
//  4. can run with tools (e.g., valgrind) to look for other errors.
//
// while the fake-pi is laughably simple and ignores more than it handles,
// it's still useful b/c:
//  1. most of your code is just straight C code, which runs the same on
//     your laptop and the pi.
//  2. the main pi specific thing is what happens when you read/write
//     gpio addresses.   given how simple these are, we can get awa
//     with just treating them as memory, where the next read returns
//     the value of the last write.  for fancier hardware, or for 
//     a complete set of possible GPIO behaviors we would have to do
//     something fancier.   
//
//     NOTE: this problem of how to model devices accurately is a 
//     big challenge both for virtual machines and machine simulators.
//
// it's good to understand what is going on here.  both why it works,
// and when you can use this kind of trick in other places.
//      - one interesting thing: we can transparently take code that
//      you wrote explicitly to run bare-metal on the pi ARM cpu,
//      and interact with the weird broadcom chip and run it on
//      your laptop, which has neither, *without making any change!*
//
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

// there is no assembly or other weird things in rpi.h so we can
// include and use it on your laptop even tho it is intended for
// the pi.
#include "rpi.h"

// macro tricks to make error reporting easier.
#define output(msg, args...) printf(msg, ##args )

#define panic(msg, args...) do {                                         \
    printf("PANIC:%s:%s:%d:", __FILE__, __FUNCTION__, __LINE__);   \
    printf(msg, ##args);                                                \
    exit(1);                                                        \
} while(0)

// main pi-specific thing is a tiny model of device
// memory: for each device address, what happens when you
// read or write it?   in real life you would build this
// model more succinctly with a map, but we write everything 
// out here for maximum obviousness.

#define GPIO_BASE 0x20200000
enum {
    gpio_fsel0 = (GPIO_BASE + 0x00),
    gpio_fsel1 = (GPIO_BASE + 0x04),
    gpio_fsel2 = (GPIO_BASE + 0x08),
    gpio_set0  = (GPIO_BASE + 0x1C),
    gpio_clr0  = (GPIO_BASE + 0x28),
    gpio_lev0  = (GPIO_BASE + 0x34)
};

static unsigned 
        gpio_fsel0_v,
        gpio_fsel1_v,
        gpio_fsel2_v,
        gpio_set0_v,
        gpio_clr0_v;

void put32(volatile void *addr, uint32_t v) {
    PUT32((uint32_t)(uint64_t)addr, v);
}

// same, but takes <addr> as a uint32_t
void PUT32(uint32_t addr, uint32_t v) {
    output("PUT32(0x%08x) = 0x%08x\n", addr, v);
    switch(addr) {
    case gpio_fsel0: gpio_fsel0_v = v;  break;
    case gpio_fsel1: gpio_fsel1_v = v;  break;
    case gpio_fsel2: gpio_fsel2_v = v;  break;
    case gpio_set0:  gpio_set0_v  = v;  break;
    case gpio_clr0:  gpio_clr0_v  = v;  break;
    case gpio_lev0:  panic("illegal write to gpio_lev0!\n");
    default: panic("write to illegal address: 0x%08x\n", addr);
    }
}

uint32_t get32(const volatile void *addr) {
    return GET32((uint32_t)(uint64_t)addr);
}

// same but takes <addr> as a uint32_t
uint32_t GET32(uint32_t addr) {
    unsigned v;
    switch(addr) {
    case gpio_fsel0: v = gpio_fsel0_v; break;
    case gpio_fsel1: v = gpio_fsel1_v; break;
    case gpio_fsel2: v = gpio_fsel2_v; break;
    case gpio_set0:  v = gpio_set0_v;  break;
    case gpio_clr0:  v = gpio_clr0_v;  break;
    // to fake a changing environment, we want gpio_lev0 to 
    // change --- so we return a random value for (which
    // will be roughly uniform random for a given bit).
    // you could bias these as well or make them more 
    // realistic by reading from a trace from a run on 
    // the raw hardware, correlating with other pins or 
    // time or ...
    case gpio_lev0:  v = random();  break;
    default: panic("read of illegal address: 0x%08x\n", addr);
    }
    output("GET32(0x%08x) = 0x%08x\n", addr,v);
    return v;
}

// don't need to do anything.  would be better to not have
// delay() in the header so we could do our own version and
// (for example) print out the number of ticks used.
void nop(void) {
}

// initialize "device memory" and then call the pi program
int main(int argc, char *argv[]) {
    void notmain(void);

    // initialize "device memory" to random values.
    // extension: do multiple iterations, initializing to
    // different values each time.
    gpio_fsel0_v = random();
    gpio_fsel1_v = random();
    gpio_fsel2_v = random();
    gpio_set0_v  = random();
    gpio_clr0_v  = random();

    // extension: run in a subprocess to isolate
    // errors.
    output("calling pi code\n");
    notmain();
    output("pi exited cleanly\n");
    return 0;
}
