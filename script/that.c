#define _GNU_SOURCE
#include <dlfcn.h>
#include <fcntl.h>
#include <stdio.h>

static int teefd = -1;
static int is_tty = -1;
static size_t (*read_original)(int, void *, size_t);

__attribute__((constructor))
void ctor(void)
{
    teefd = open(
        "/tmp/log-more-stdin.log",
        O_CREAT|O_WRONLY|O_TRUNC, 0666);

    // resolve symbol read
    read_original = dlsym(RTLD_NEXT, "read");
}

size_t read(int fd, void *buffer, size_t size)
{
    // invoke original read(...)
    size_t count = read_original(fd, buffer, size);

    // if fd is STDIN and teefd is open, echo input
    if(fd == 0 && teefd != -1 && isatty(fd))
    {
        write(teefd, buffer, count);
        fsync(teefd);
    }

    // return result of read_original(...)
    return count;
}
