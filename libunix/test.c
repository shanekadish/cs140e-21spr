#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

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


int main(void) {
    unsigned size;
    char *buf = read_file(&size, "test.txt");
    for (int i = 0; buf[i]; i++)
        printf("%c", buf[i]);
    return 0;
}