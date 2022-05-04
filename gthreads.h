#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <ucontext.h>
#include <sys/time.h>

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
int gthread_run(void *func);
void gthread_join(int thread);
void gthread_mutex_init(gthread_mutex *mutex);
void gthread_mutex_unlock(gthread_mutex *mutex);