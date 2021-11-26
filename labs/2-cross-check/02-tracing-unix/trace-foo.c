// trivial program to show how ld function wrapping works.
#include <stdio.h>
#include "foo.h"

// gross: ld creates these but we have to define by hand.  be very
// careful!  you could do some macro tricks, but we keep it simple.
unsigned __wrap_foo(unsigned arg0, unsigned arg1);
unsigned __real_foo(unsigned arg0, unsigned arg1);

// wrapper inserted by ld (see makefile)
unsigned __wrap_foo(unsigned arg0, unsigned arg1) {
    printf("in wrap foo(%u,%u)\n", arg0,arg1);

    // show that we can mess with the arguments and forward them.
    arg0++;
    arg1++;
    printf("calling real foo(%u,%u)\n", arg0,arg1);
    unsigned ret = foo(arg0,arg1);
    printf("returning from wrap_foo\n");
    return ret;
}

int main(void) {
    unsigned a0 = 1, a1 = 2;
    printf("about to call __wrap_foo(%d,%d)\n", a0,a1);
    unsigned ret = __wrap_foo(a0,a1);
    printf("returned  %u\n", ret);
    return 0;
}
