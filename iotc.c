#include <stdio.h>
#include <stdlib.h>
#include <libaio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>

/* dispatch 3 4k-size ios using the io_type specified by user */
#define NUM_EVENTS  3
#define ALIGN_SIZE  4096
#define WR_SIZE  4096

enum io_type {
    SEQUENCE_IO,
    REVERSE_IO,
    INTERLEAVE_IO,
    IO_TYPE_END
};

int io_units[IO_TYPE_END][NUM_EVENTS] = 
{
    {0, 1, 2}, /* corresponding to SEQUENCE_IO */
    {2, 1, 0}, /* corresponding to REVERSE_IO */
    {2, 0, 1}  /* corresponding to INTERLEAVE_IO */
};

char *io_opt = "srid:"; /* acceptable options */

int main(int argc, char *argv[])
{
    int fd;
    io_context_t ctx;
    struct timespec tms;
    struct io_event events[NUM_EVENTS];
    struct iocb iocbs[NUM_EVENTS], *iocbp[NUM_EVENTS];
    int i, r;
    void *buf;
    int io_flag = -1;
    bool hit = false;
    char *dev = NULL, opt;

    /* parse options */
    while ((opt = getopt(argc, argv, io_opt)) != -1) {
        switch (opt) {
        case 's':
            if (hit) {
                fprintf(stderr, "Conflict options! \n");                
                goto USAGE;
            }
            io_flag = SEQUENCE_IO;
            hit = true;
            break;
        case 'r':
            if (hit) {
                fprintf(stderr, "Conflict options! \n");                
                goto USAGE;
            }    
            io_flag = REVERSE_IO;
            hit = true;
            break;
        case 'i':
            if (hit) {
                fprintf(stderr, "Conflict options! \n");                
                goto USAGE;
            }
            io_flag = INTERLEAVE_IO;
            hit = true;
            break;
        case 'd':
            dev = optarg;
            break;
       default: /* '?' */
USAGE:
            fprintf(stderr, "Usage: %s [-d Device] [-s | -r | -i] \n\n eg: iotc -d /dev/sdb -s \n\n"
                    "-d specify a block device to submit IOS, device name must follow\n"
                    "-s conflict with -s and -i, submit sequence IOs, such as 0+8, 8+8, 16+8\n"
                    "-r conflict with -s and -i, submit reverse IOs, such as 16+8, 8+8, 0+8\n"
                    "-i conflict with -s and -r, submit interleave IOs, such as 16+8, 0+8, 8+8\n",
                    argv[0]);
            return 2;
        }
    }

    if (!hit || !dev)
        goto USAGE;

    fd = open(dev, O_RDWR|__O_DIRECT);

    if (fd <= 0) {
        fprintf(stderr, "open %s err! please make sure you have specified right device and permission to access!",
                dev);
        return 3;
    }
    
    /* we can dispatch 32 IOs at one systemcall */
    ctx = 0;
    if (io_setup(32, &ctx)) {
        perror("io_setup");
        goto OUT1;
    }

    if (posix_memalign(&buf, ALIGN_SIZE, WR_SIZE)) {
        perror("posix_memalign");
        goto OUT0;
    }

    /* prepare IO request according to io_type */
    for (i = 0; i < NUM_EVENTS; ++i) {
        iocbp[i] = iocbs + i;
        io_prep_pwrite(&iocbs[i], fd, buf, WR_SIZE, io_units[io_flag][i] * WR_SIZE);
	}
    /* submit IOs using io_submit systemcall */
    if (io_submit(ctx, NUM_EVENTS, iocbp) != NUM_EVENTS) {
        perror("io_submit");
        goto OUT0;
    }
    
    /* get the IO result with a timeout of 1 second */
    tms.tv_sec = 1;
    tms.tv_nsec = 0;
    r = io_getevents(ctx, 1, NUM_EVENTS, events, &tms);
    if (r > 0) {
        for (i = 0; i < r; ++i)
            printf("request(offset:%lld) had wroten %lu bytes!\n", (events[i].obj)->u.c.offset, (events[i].obj)->u.c.nbytes);
    }
    
OUT0:
    free(buf);
OUT1:
    io_destroy(ctx);
    close(fd);
    return 0;
}
