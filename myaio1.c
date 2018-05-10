#include <stdio.h>
#include <errno.h>
#include <libaio.h>
#include <sys/eventfd.h>
#include <sys/epoll.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#define TEST_FILE   "/dev/sdb"
#define NUM_EVENTS  5
#define ALIGN_SIZE  4096
#define RD_WR_SIZE  4096

struct custom_iocb
{
    struct iocb iocb;
    int nth_request;
};

void aio_callback(io_context_t ctx, struct iocb *iocb, long res, long res2)
{
    struct custom_iocb *iocbp = (struct custom_iocb *)iocb;
    printf("nth_request: %d, request_type: %s, offset: %lld, length: %lu, res: %ld, res2: %ld\n", 
            iocbp->nth_request, (iocb->aio_lio_opcode == IO_CMD_PREAD) ? "READ" : "WRITE",
            iocb->u.c.offset, iocb->u.c.nbytes, res, res2);

}

int main(int argc, char *argv[])
{
    int fd;
    io_context_t ctx;
    struct timespec tms;
    struct io_event events[NUM_EVENTS];
    struct custom_iocb iocbs[NUM_EVENTS];
    struct iocb *iocbps[NUM_EVENTS];
    struct custom_iocb *iocbp;
    int i, j, r;
    void *buf;


    //fd = open(TEST_FILE, O_RDWR | O_CREAT | O_DIRECT, 0644);
    fd = open(TEST_FILE, O_RDWR|__O_DIRECT);


    if (fd == -1) {
        perror("open");
        return 3;
    }
    
    ctx = 0;
    if (io_setup(8192, &ctx)) {
        perror("io_setup");
        return 4;
    }

    if (posix_memalign(&buf, ALIGN_SIZE, RD_WR_SIZE)) {
        perror("posix_memalign");
        return 5;
    }
    printf("buf: %p\n", buf);

    for (i = 0, iocbp = iocbs; i < NUM_EVENTS; ++i, ++iocbp) {
        iocbps[i] = &iocbp->iocb;

/*
	if(i == 1)
        	io_prep_pwrite(&iocbp->iocb, fd, buf, RD_WR_SIZE, 2 * RD_WR_SIZE);
	else if (i == 2)
        	io_prep_pwrite(&iocbp->iocb, fd, buf, RD_WR_SIZE, 5 * RD_WR_SIZE);
	else
        	io_prep_pwrite(&iocbp->iocb, fd, buf, RD_WR_SIZE, 8 * RD_WR_SIZE);
*/
	io_prep_pwrite(&iocbp->iocb, fd, buf, RD_WR_SIZE, ((i + 1)*3 - 1) * RD_WR_SIZE);	

        io_set_callback(&iocbp->iocb, aio_callback);
        iocbp->nth_request = i + 1;
    }

usleep(100);

    if (io_submit(ctx, NUM_EVENTS, iocbps) != NUM_EVENTS) {
        perror("io_submit");
        return 6;
    }


    tms.tv_sec = 0;
    tms.tv_nsec = 0;
    //r = io_getevents(ctx, 1, NUM_EVENTS, events, &tms);
    r = io_getevents(ctx, 1, NUM_EVENTS, events, NULL);
    printf("r=%d\n", r);
    if (r > 0) {
        for (j = 0; j < r; ++j) {
            ((io_callback_t)(events[j].data))(ctx, events[j].obj, events[j].res, events[j].res2);
        }
    }
    
    free(buf);
    io_destroy(ctx);
    close(fd);

    return 0;
}
