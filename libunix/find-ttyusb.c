// engler, cs140e: your code to find the tty-usb device on your laptop.
#include <assert.h>
#include <fcntl.h>
#include <string.h>

#include "libunix.h"

#define _SVID_SOURCE
#include <dirent.h>
static const char *ttyusb_prefixes[] = {
	"ttyUSB",	// linux
	"cu.usbserial", // mac os
    // if your system uses another name, add it.
	0
};

static int filter(const struct dirent *d) {
    const char *prefix;
    for (int i = 0; (prefix = ttyusb_prefixes[i]); i++)
        if (!strncmp(d->d_name, prefix, strlen(prefix))) return 1;
    return 0;
}

// find the TTY-usb device (if any) by using <scandir> to search for
// a device with a prefix given by <ttyusb_prefixes> in /dev
// returns:
//  - device name.
// error: panic's if 0 or more than 1 devices.
char *find_ttyusb(void) {
    // TODO: "return a malloc'd name so doesn't corrupt"
    struct dirent **dirents;
    int nfiles = scandir("/dev", &dirents, filter, alphasort);
    if (nfiles == -1) perror("find_ttyusb: scandir failed");
    if (nfiles != 1) panic("Found dodgy number of dirent matches: %d\n", nfiles);
    return dirents[0]->d_name;
}

// return the most recently mounted ttyusb (the one
// mounted last).  use the modification time 
// returned by state.
// TODO: Check that default order is modification time, else define `compar` function
char *find_ttyusb_last(void) {
    struct dirent **dirents;
    int nfiles = scandir("/dev", &dirents, filter, NULL);
    if (nfiles == -1) perror("find_ttyusb: scandir failed");
    return dirents[nfiles - 1]->d_name;
}

// return the oldest mounted ttyusb (the one mounted
// "first") --- use the modification returned by
// stat()
// TODO: Check that default order is modification time, else define `compar` function
char *find_ttyusb_first(void) {
    return find_ttyusb();
}
