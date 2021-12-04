#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "libunix.h"

// read in file <name>
// returns:
//  - pointer to the code.  pad code with 0s up to the
//    next multiple of 4.  
//  - bytes of code in <size>
//
// fatal error open/read of <name> fails.
// 
// How: 
//    - use stat to get the size of the file.
//    - round up to a multiple of 4.
//    - allocate a buffer  --- 
//    - zero pads to a // multiple of 4.
//    - read entire file into buffer.  
//    - make sure any padding bytes have zeros.
//    - return it.   
//
// make sure to close the file descriptor (this will
// matter for later labs).
void *read_file(unsigned *size, const char *name) {
    assert(name != NULL);

    struct stat stat_buf;
    if (stat(name, &stat_buf) == -1) perror("read_file: stat failed");

    int fd = open(name, O_RDONLY);
    if (fd == -1) perror("read_file: open failed");

    // TODO: This adds 4 extra pad bytes in cases where the code size is already
    //       a multiple of 4. Check if this is correct.
    size_t nchars = stat_buf.st_size + (4 - stat_buf.st_size % 4);
    char *buf = calloc(nchars, sizeof(char));
    if (buf == NULL) perror("read_file: calloc failed");

    *size = read(fd, buf, nchars);
    if (*size == -1) perror("read_file: read failed");

    close(fd);
    return buf;
}