/* 
 * engler, cs140e: simple unix-side bootloader implementation.  
 * see the lab README.md for the protocol definition.
 *
 * all your modifications should go here.
 */
#include "libunix.h"
#include "simple-boot.h"

// Implement steps
//    1,2,3,4.
//
//  0 and 5 are implemented as demonstration.
//
// Note: if timeout in <set_tty_to_8n1> is too small (set by our caller)
// you can fail here. when you do a read and the pi doesn't send data quickly enough.
void simple_boot(int fd, const uint8_t *buf, unsigned n) { 
    // if you want to trace PUT/GET set
    trace_p = 1;

    if(pi_roundup(n,4) % 4 != 0)
        panic("boot code size (%d bytes) is not a multiple of 4!\n", n);

    // all implementations should have the same message: same bytes,
    // same crc32: cross-check these values to detect if your <read_file> 
    // is busted.
    trace("simple_boot: sending %d bytes, crc32=%x\n", n, crc32(buf,n));

    output("waiting for a start\n");
    uint32_t op;

    // we drain the initial pipe: can have garbage.  the code is a bit odd b/c 
    // if we have a single garbage byte, then reading 32-bits will
    // will not match <GET_PROG_INFO> (obviously) and if we keep reading 
    // 32 bits then we will never sync up with the input stream, our hack
    // is to read a byte (to try to sync up) and then go back to reading 32-bit
    // ops.
    //
    // CRUCIAL: make sure you use <get_op> for the first word in each 
    // message.
    while((op = get_op(fd)) != GET_PROG_INFO) {
        output("expected initial GET_PROG_INFO, got <%x>: discarding.\n", op);
        // have to remove just one byte since if not aligned, stays not aligned
        get_uint8(fd);
    } 

    // 1. reply to the GET_PROG_INFO
    trace_put32(fd, PUT_PROG_INFO);
    trace_put32(fd, ARMBASE);
    trace_put32(fd, n);
    trace_put32(fd, crc32(buf, n));

    // 2. drain any extra GET_PROG_INFOS
    while((op = get_op(fd)) == GET_PROG_INFO)
        output("draining spurious GET_PROG_INFO, got <%x>: discarding.\n", op);

    // 3. check that we received a GET_CODE
    assert(op == GET_CODE);
    output("Got <%x> (GET_CODE)\n", op);
    assert((op = get_op(fd)) == crc32(buf, n));
    output("CRC from pi (%x) matches CRC from unix (%x), yay!\n", op, crc32(buf, n));

    // 4. handle it: send a PUT_CODE + the code.
    trace_put32(fd, PUT_CODE);
    output("Sending program code to pi...\n");
    for (int i = 0; i < n; i++)
        put_uint8(fd, buf[i]);

    // 5. Wait for BOOT_SUCCESS
    ck_eq32(fd, "BOOT_SUCCESS mismatch", BOOT_SUCCESS, get_op(fd));
    output("bootloader: Done.\n");
}
