// trivial little fake r/pi system that will allow you to debug your
// gpio.c code. 
#include <stdbool.h>

#include "fake-pi.h"

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

typedef uint32_t word_t;

typedef struct mem {
    uint32_t addr;
    uint32_t value;
    struct mem *next;
} mem_t;

typedef struct mem_list {
    mem_t *head;
    mem_t *tail;
    uint32_t size;
} mem_list_t;

static mem_list_t *mem_list = NULL;

static mem_t *mem_init(uint32_t addr, uint32_t value) {
    mem_t *mem = malloc(sizeof(mem_t));
    assert(mem != NULL);
    mem->addr = addr;
    mem->value = value;
    mem->next = NULL;
    return mem;
}

static mem_list_t *mem_list_init(void) {
    mem_list_t *mem_list = malloc(sizeof(mem_list_t));
    assert(mem_list != NULL);
    mem_list->head = NULL;
    mem_list->tail = NULL;
    mem_list->size = 0;
    return mem_list;
}

static bool mem_list_contains(mem_list_t *mem_list, uint32_t addr) {
    assert(mem_list != NULL);
    for (mem_t *current = mem_list->head; current != NULL; current = current->next)
        if (current->addr == addr) return true;
    return false;
}

static mem_t *mem_list_get(mem_list_t *mem_list, uint32_t addr) {
    assert(mem_list != NULL);
    for (mem_t *current = mem_list->head; current != NULL; current = current->next)
        if (current->addr == addr) return current;
    return NULL;
}

static bool mem_list_is_empty(mem_list_t *mem_list) {
    assert(mem_list != NULL);
    return mem_list->size == 0;
}

static mem_t *mem_list_append(mem_list_t *mem_list, uint32_t addr, uint32_t value) {
    assert(mem_list != NULL);
    mem_t *mem = mem_init(addr, value);
    assert(mem != NULL);
    if (mem_list_is_empty(mem_list)) {
        mem_list->head = mem_list->tail = mem;
    } else {
        mem_list->tail->next = mem;
        mem_list->tail = mem;
    }
    mem_list->size++;
    return mem;
}

static void mem_list_update(mem_list_t *mem_list, uint32_t addr, uint32_t value) {
    assert(mem_list_contains(mem_list, addr));
    mem_t *mem = mem_list_get(mem_list, addr);
    assert(mem != NULL);
    mem->value = value;
}

static unsigned int get_gpio_fsel_paddr(unsigned int pin) {
    return gpio_fsel0 + ((pin / 10) * sizeof(word_t));
}

void gpio_set_function(unsigned pin, gpio_func_t function) {
    uint32_t fsel_paddr = get_gpio_fsel_paddr(pin);
    uint32_t old_value = GET32(fsel_paddr);
    uint32_t bits_to_clear = 0b111 << ((pin % 10) * 3);
    uint32_t bits_to_set = function << ((pin % 10) * 3);
    uint32_t new_value = (old_value & ~bits_to_clear) | bits_to_set;
    PUT32(fsel_paddr, new_value);
}

void put32(volatile void *addr, uint32_t v) {
    PUT32((uint32_t)(uint64_t)addr, v);
}

// same, but takes <addr> as a uint32_t
void PUT32(uint32_t addr, uint32_t v) {
    if (mem_list == NULL) mem_list = mem_list_init();
    switch(addr) {
    case gpio_lev0:  panic("illegal write to gpio_lev0!\n");
    default: 
        output("store (addr,v) for later lookup\n");
        if (mem_list_contains(mem_list, addr)) mem_list_update(mem_list, addr, v);
        else mem_list_append(mem_list, addr, v);
    }
    trace("PUT32:%x:%x\n", addr,v);
}

uint32_t get32(const volatile void *addr) {
    return GET32((uint32_t)(uint64_t)addr);
}

/*
 * most <addr> we treat as normal memory: return the 
 * value of the last write.
 *
 * otherwise we do different things: time, status regs,
 * input pins.
 */
uint32_t GET32(uint32_t addr) {
    if (mem_list == NULL) mem_list = mem_list_init();
    unsigned v;

    switch(addr) {
    // to fake a changing environment, we want gpio_lev0 to 
    // change --- we just use a uniform random coin toss, 
    // but you would bias these as well or make them more 
    // realistic by reading from a trace from a run on 
    // the raw hardware, correlating with other pins or 
    // time or ...
    case gpio_lev0:  v = fake_random();  break;
    default: 
        // return value of last write, or random if this is the first read.
        if (mem_list_contains(mem_list, addr)) v = mem_list_get(mem_list, addr)->value;
        else v = mem_list_append(mem_list, addr, fake_random())->value;
        break;
    }
    trace("GET32:%x:%x\n", addr,v);
    return v;
}
