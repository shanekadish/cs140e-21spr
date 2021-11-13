/*
 * Implement the following routines to set GPIO pins to input or output,
 * and to read (input) and write (output) them.
 *
 * DO NOT USE loads and stores directly: only use GET32 and PUT32 
 * to read and write memory.  Use the minimal number of such calls.
 *
 * See rpi.h in this directory for the definitions.
 */
#include "rpi.h"

// see broadcomm documents for magic addresses.
#define GPIO_BASE_PADDR 0x20200000
static const unsigned int gpio_fsel0 = (GPIO_BASE_PADDR);
static const unsigned int gpio_set0  = (GPIO_BASE_PADDR + 0x1C);
static const unsigned int gpio_clr0  = (GPIO_BASE_PADDR + 0x28);
static const unsigned int gpio_lev0  = (GPIO_BASE_PADDR + 0x34);

//
// Part 1 implement gpio_set_on, gpio_set_off, gpio_set_output
//

// set <pin> to be an output pin.  
//
// note: fsel0, fsel1, fsel2 are contiguous in memory, so you 
// can (and should) use array calculations!
void gpio_set_output(unsigned int pin) {
    // implement this
    // use <gpio_fsel0>
}

// set GPIO <pin> on.
void gpio_set_on(unsigned int pin) {
    // implement this
    // use <gpio_set0>
}

// set GPIO <pin> off
void gpio_set_off(unsigned int pin) {
    // implement this
    // use <gpio_clr0>
}

// set <pin> to <v> (v \in {0,1})
void gpio_write(unsigned int pin, unsigned int v) {
    if(v)
       gpio_set_on(pin);
    else
       gpio_set_off(pin);
}

//
// Part 2: implement gpio_set_input and gpio_read
//

// set <pin> to input.
void gpio_set_input(unsigned int pin) {
    // implement.
}

// return the value of <pin>
int gpio_read(unsigned int pin) {
    unsigned int v = 0;

    // implement.
    return v;
}
