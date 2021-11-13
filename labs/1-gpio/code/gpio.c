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

typedef uint32_t word_t;

/*
 * Engler:
 * "The different GPFSELn registers handle groups of 10, so you can divide the
 * pin number to compute the right GPFSEL register"
 *
 * gpio_fsel0 = { GPIO  0 ... GPIO  9 }
 * gpio_fsel1 = { GPIO 10 ... GPIO 19 }
 * gpio_fsel2 = { GPIO 20 ... GPIO 29 }
 * ...
 * gpio_fsel5 = { GPIO 50 ... GPIO 53 }
 *
 * TODO: How to make sense of the fact that we're on an arch with only a subset of these GPIO pins?
 */
static unsigned int get_gpio_fsel_paddr(unsigned int pin) {
    return gpio_fsel0 + ((pin / 10) * sizeof(word_t));
}

//
// Part 1 implement gpio_set_on, gpio_set_off, gpio_set_output
//

// set <pin> to be an output pin.  
//
// note: fsel0, fsel1, fsel2 are contiguous in memory, so you 
// can (and should) use array calculations!
void gpio_set_output(unsigned int pin) {
    // Write '001' to bits [k .. k - 2] of gpio_fseln, **leaving all other bits undisturbed**
    // TODO: I know that Engler alluded to the undisturbed thing, but where in the broadcom doc was this mentioned?
    // TODO: Anyone know WTF is going on in the hardware that the "unused" bits need their values preserved? Are they used as scratch space or some shit?
    uint32_t fsel_paddr = get_gpio_fsel_paddr(pin);
    uint32_t old_value = GET32(fsel_paddr);
    uint32_t bits_to_clear = 0b111 << ((pin % 10) * 3);
    uint32_t bits_to_set = 0b001 << ((pin % 10) * 3);
    uint32_t new_value = (old_value & ~bits_to_clear) | bits_to_set;
    PUT32(fsel_paddr, new_value);
}

// set GPIO <pin> on.
void gpio_set_on(unsigned int pin) {
    PUT32(gpio_set0, (1 << pin));
}

// set GPIO <pin> off
void gpio_set_off(unsigned int pin) {
    PUT32(gpio_clr0, (1 << pin));
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
    uint32_t fsel_paddr = get_gpio_fsel_paddr(pin);
    uint32_t old_value = GET32(fsel_paddr);
    uint32_t bits_to_clear = 0b111 << ((pin % 10) * 3);
    uint32_t new_value = old_value & ~bits_to_clear;
    PUT32(fsel_paddr, new_value);
}

// return the value of <pin>
int gpio_read(unsigned int pin) {
    return (GET32(gpio_lev0) & (1 << pin)) >> pin;
}
