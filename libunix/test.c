#include <stdio.h>
#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void *read_file(unsigned *size, const char *name) {
    assert(name != NULL);

    struct stat stat_buf;

    int ret = stat(name, &stat_buf);
    if (ret == -1) perror("read_file: stat failed");

    int fd = open(name, O_RDONLY);
    if (fd == -1) perror("read_file: open failed");

    size_t nchars = stat_buf.st_size + (4 - stat_buf.st_size % 4);
    char *buf = calloc(nchars, sizeof(char));
    if (buf == NULL) perror("read_file: calloc failed");

    *size = read(fd, buf, nchars);
    if (*size == -1) perror("read_file: read failed");

    close(fd);
    return buf;
}

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

char *find_ttyusb(void) {
    struct dirent **dirents;
    int nfiles = scandir("/dev", &dirents, filter, alphasort);
    if (nfiles == -1) perror("find_ttyusb: scandir failed");
    if (nfiles == 0 || nfiles > 1) fprintf(stderr, "Found dodgy number of dirent matches: %d\n", nfiles);
    return dirents[0]->d_name;
}

int main(void) {
    printf("%s\n", find_ttyusb());
    return 0;
}