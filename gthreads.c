#include "gthreads.h"

gthread_mutex mut;

gthread threads[MAX_THREADS];
gthread *thread_current;
int thread_currrent_id = 0;

ucontext_t interrupt_context;
unsigned char *interrupt_stack;
sigset_t interrupt_sigmask;

ucontext_t finish_contexts[MAX_THREADS];

int first_time = 0;
struct itimerval timer;
static int pos = 1;

void gthread__schedule(){
    #ifdef DEBUG
    printf("On interrupt, thread ID: %d\n", thread_currrent_id);

    printf("-----------\n");
    for(int i=0;i<5;i++){
        printf("%d ", threads[i].state);
    }
    printf("\n-----------\n");

    #endif

    do{
        thread_currrent_id = (thread_currrent_id + 1) % MAX_THREADS;
    }while(threads[thread_currrent_id].state != Ready );

    if(thread_current->state != Finished){
        thread_current->state = Ready;
    }

    thread_current = &threads[thread_currrent_id];
    thread_current->state = Running;

    #ifdef DEBUG
    printf("On interrupt exit, thread ID to be run: %d\n", thread_currrent_id);
    #endif

    swapcontext(&interrupt_context, &thread_current->context);
}

void gthread__timer_interrupt_handler(int warn, siginfo_t *sig, void *prev_context){
    #ifdef DEBUG
    printf("On handler..\n");
    #endif

    if(thread_current->state != Finished){
        thread_current->state = Ready;
    }

    getcontext(&interrupt_context);
    interrupt_context.uc_stack.ss_sp = interrupt_stack;
    interrupt_context.uc_stack.ss_size = STACK_SIZE;

    sigemptyset(&interrupt_context.uc_sigmask);
    makecontext(&interrupt_context, gthread__schedule, 0);

    swapcontext(&thread_current->context, &interrupt_context);
}

void gthread__setup_signal(){
    struct sigaction action;

    memset(&action, 0, sizeof(action));
    action.sa_sigaction = gthread__timer_interrupt_handler;
    sigemptyset(&action.sa_mask);

    sigemptyset(&interrupt_sigmask);
    sigaddset(&interrupt_sigmask, SIGALRM);

    sigaction(SIGALRM, &action, NULL);
}

void gthread__pause_timer(){
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;
    timer.it_value = timer.it_interval;
    setitimer(ITIMER_REAL, &timer, 0);
}

void gthread__resume_timer(){
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = NANOSECONDS * 1000;
    timer.it_value = timer.it_interval;
    setitimer(ITIMER_REAL, &timer, NULL);
}

void gthread__finish_runner(){
    threads[thread_currrent_id].state = Finished;
    #ifdef DEBUG
    printf("COUNTER: %d\n", cn);
    #endif
}

void gthread_init(void){
    for(int i=0; i<MAX_THREADS; i++){
        threads[i].state = Unused;
        threads[i].waiting_on = -1;
    }

    interrupt_stack = malloc(STACK_SIZE);
    if(interrupt_stack == NULL){
        exit(1);
    }

    gthread__setup_signal();
}

int gthread_run(void *func, int arg1, int arg2, int arg3){
    unsigned char *stack, *finish_stack;
    gthread *thr;

    if(pos >= MAX_THREADS){
        exit(-1);
    }

    while(pos < MAX_THREADS){
        if(threads[pos].state == Unused){
            thr = &threads[pos];
            pos++;
            break;
        }
        pos++;
    }

    stack = malloc(STACK_SIZE);
    if (!stack){
        exit(-1);
    }
    getcontext(&thr->context);

    thr->context.uc_stack.ss_sp = stack;
    thr->context.uc_stack.ss_size = STACK_SIZE;
    thr->context.uc_stack.ss_flags = 0;
    thr->context.uc_link = &finish_contexts[pos - 1];

    sigemptyset(&thr->context.uc_sigmask);

    makecontext(&thr->context, func, 3, arg1, arg2, arg3);
    thr->state = Ready;

    /////////////////////////////////////
    finish_stack = malloc(STACK_SIZE/8);
    if (!finish_stack){
        exit(-1);
    }
    getcontext(&finish_contexts[pos - 1]);

    finish_contexts[pos - 1].uc_stack.ss_sp = finish_stack;
    finish_contexts[pos - 1].uc_stack.ss_size = STACK_SIZE/8;
    finish_contexts[pos - 1].uc_stack.ss_flags = 0;
    finish_contexts[pos - 1].uc_link = &threads[0].context;

    sigemptyset(&finish_contexts[pos - 1].uc_sigmask);

    makecontext(&finish_contexts[pos - 1], gthread__finish_runner, 0);

    if (first_time == 0){
        getcontext(&threads[0].context);

        thread_current = &threads[0];
        threads[0].state = Running;

        timer.it_interval.tv_sec = 0;
        timer.it_interval.tv_usec = NANOSECONDS * 1000;
        timer.it_value = timer.it_interval;

        setitimer(ITIMER_REAL, &timer, NULL);
        first_time = 1;
    }
    return pos - 1;
}

void gthread_join(int thread){
    // printf("THREAD: %d\n", thread);
    // for(int i=0;i<10;i++){
    //     printf("State: %d, Waiting on: %d\n", threads[i].state, threads[i].waiting_on);
    // }

    if (threads[thread].waiting_on == -1 && threads[thread].state == Ready){
        gthread__pause_timer();
        swapcontext(&threads[0].context, &threads[thread].context);
        gthread__resume_timer();
    }
    else if(threads[thread].waiting_on != -1){
        gthread__pause_timer();
        swapcontext(&threads[0].context, &threads[threads[thread].waiting_on].context);
        gthread__resume_timer();
    }
}

void gthread_mutex_init(gthread_mutex *mutex){
    mutex->owner = -1;
}

void gthread_mutex_lock(gthread_mutex *mutex){
    if(mutex->owner == -1){
        mutex->owner = thread_currrent_id;
    }
    else{
        //schedule another and "wait"
        while(mutex->owner != -1){
            thread_current->waiting_on = mutex->owner;

            getcontext(&interrupt_context);
            interrupt_context.uc_stack.ss_sp = interrupt_stack;
            interrupt_context.uc_stack.ss_size = STACK_SIZE;

            sigemptyset(&interrupt_context.uc_sigmask);
            makecontext(&interrupt_context, gthread__schedule, 0);

            swapcontext(&thread_current->context, &interrupt_context);
        }
        mutex->owner = thread_currrent_id;
        thread_current->waiting_on = -1;
    }
}

void gthread_mutex_unlock(gthread_mutex *mutex){
    if(mutex->owner == thread_currrent_id){
        mutex->owner = -1;
    }
}
