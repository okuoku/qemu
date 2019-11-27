/* FIXME: Cygwin glue codes */

#include <windows.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>


typedef void WaitObjectFunc(void *opaque);
typedef void IOHandler(void* opaque);
void qemu_set_fd_handler(int fd, IOHandler *fd_read, IOHandler *fd_write, void *opaque);

int net_init_bridge(void){
    return -1;
}
int tap_get_fd(void){ /* FIXME */
    return -1;
}

typedef struct {
    HANDLE h;
    int fd_write;
    int fd_read;
    WaitObjectFunc* fn;
    void* ptr;
} waiter_ctx;

static void
handle_waiter_read(void *p){
    char x[256];
    int r;
    waiter_ctx* ctx = (waiter_ctx*)p;
    r = read(ctx->fd_read, x, 1);
    //printf("NET Recv... %d\n",r);
    ctx->fn(ctx->ptr);
}

static void*
thr_waiter(void* c){
    DWORD d;
    waiter_ctx* ctx = (waiter_ctx*)c;

    for(;;){
        d = WaitForSingleObject(ctx->h, INFINITE);
        if(d){
            close(ctx->fd_write);
            return NULL;
        }
        //printf("NET Signal...\n");
        write(ctx->fd_write, &d, 1);
    }
}

int qemu_add_wait_object(HANDLE handle, WaitObjectFunc *func, void *opaque)
{
    int fds[2];
    pthread_t thr;
    waiter_ctx* ctx;
    ctx = malloc(sizeof(waiter_ctx));

    pipe(fds);

    ctx->fd_read = fds[0];
    ctx->fd_write = fds[1];
    ctx->h = handle;
    ctx->fn = func;
    ctx->ptr = opaque;

    pthread_create(&thr, NULL, thr_waiter, ctx);

    qemu_set_fd_handler(fds[0], handle_waiter_read, NULL, ctx);

    return 0;
}

void qemu_del_wait_object(HANDLE handle, WaitObjectFunc *func, void *opaque)
{
    /* Do nothing */
}

