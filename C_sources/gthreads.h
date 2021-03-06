#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <ucontext.h>
#include <sys/time.h>
#include <stdarg.h>

#define MAX_THREADS 7510
#define STACK_SIZE 2048
#define NANOSECONDS 1000

typedef struct gthr {
    ucontext_t context;
    enum {
        Unused,
        Running,
        Ready,
        Finished
    } state;
    int waiting_on;
} gthread;

typedef struct gthr_mutex {
    int owner;
} gthread_mutex;


void gthread_init(void);
int gthread_run(void *func, int arg1, int arg2, int arg3);
void gthread_join(int thread);
void gthread_mutex_init(gthread_mutex *mutex);
void gthread_mutex_unlock(gthread_mutex *mutex);