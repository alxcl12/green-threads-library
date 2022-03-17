#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <ucontext.h>
#include <sys/time.h>

#define MAX_THREADS 128
#define STACK_SIZE 4096
#define NANOSECONDS 10

typedef struct gthr {
    ucontext_t context;
	enum {
		Unused,
		Running,
		Ready,
        Main,
        Finished
	} state;
} gthread;

typedef struct gthr_mutex {
    gthread owner;
	enum {
        unlocked,
        locked
	} state;
} gthread_mutex;


gthread threads[MAX_THREADS];
gthread *thread_current;
int thread_currrent_id = 0;

ucontext_t interrupt_context;
unsigned char *interrupt_stack;
sigset_t interrupt_sigmask;

ucontext_t finish_context;
unsigned char *finish_stack;

int cn = 0;

void gthread_schedule(){
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
    }while(threads[thread_currrent_id].state != Ready);

    if(thread_current->state != Main && thread_current->state != Finished){
        thread_current->state = Ready;
    }

    thread_current = &threads[thread_currrent_id];
    thread_current->state = Running;

    #ifdef DEBUG
    printf("On interrupt exit, thread ID to be run: %d\n", thread_currrent_id);
    #endif

    swapcontext(&interrupt_context, &thread_current->context);
}

void gthread_timer_interrupt_handler(int warn, siginfo_t *sig, void *prev_context){
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
    makecontext(&interrupt_context, gthread_schedule, 0);

    swapcontext(&thread_current->context, &interrupt_context);
}

void gthread_setup_signal(){
    struct sigaction action;

    memset(&action, 0, sizeof(action));
    action.sa_sigaction = gthread_timer_interrupt_handler;
    sigemptyset(&action.sa_mask);

    sigemptyset(&interrupt_sigmask);
    sigaddset(&interrupt_sigmask, SIGALRM);

    sigaction(SIGALRM, &action, NULL);
}

void gthread_main_runner(){
    printf("In main..\n");

    getcontext(&interrupt_context);
    interrupt_context.uc_stack.ss_sp = interrupt_stack;
    interrupt_context.uc_stack.ss_size = STACK_SIZE;

    sigemptyset(&interrupt_context.uc_sigmask);
    makecontext(&interrupt_context, gthread_schedule, 0);

    swapcontext(&thread_current->context, &interrupt_context);
}

void gthread_finish_runner(){
    threads[thread_currrent_id].state = Finished;
    cn++;

    #ifdef DEBUG
    printf("COUNTER: %d\n", cn);
    #endif

    getcontext(&threads[0].context);
    // threads[0].context.uc_stack.ss_sp = threads[0].context.;
    // threads[0].context.uc_stack.ss_size = STACK_SIZE;

    // sigemptyset(&threads[0].context.uc_sigmask);
    makecontext(&threads[0].context, gthread_main_runner, 0);

    thread_currrent_id = 0;
    swapcontext(&thread_current->context, &threads[0].context);
}

void gthread_setup_main(){
	unsigned char *stack;

	stack = malloc(STACK_SIZE);
	if (!stack){
	    exit(-1);
    }
    getcontext(&threads[0].context);

    threads[0].context.uc_stack.ss_sp = stack;
    threads[0].context.uc_stack.ss_size = STACK_SIZE;
    threads[0].context.uc_stack.ss_flags = 0;

    sigemptyset(&threads[0].context.uc_sigmask);

    makecontext(&threads[0].context, gthread_main_runner, 0);
    threads[0].state = Main;
    thread_current = &threads[0];
}

void gthread_setup_finish(){
	unsigned char *stack = finish_stack;

	stack = malloc(STACK_SIZE);
	if (!stack){
	    exit(-1);
    }
    getcontext(&finish_context);

    finish_context.uc_stack.ss_sp = stack;
    finish_context.uc_stack.ss_size = STACK_SIZE;
    finish_context.uc_stack.ss_flags = 0;
    finish_context.uc_link = &threads[0].context;

    sigemptyset(&finish_context.uc_sigmask);

    makecontext(&finish_context, gthread_finish_runner, 0);
}

void gthread_init(void){
    for(int i=0; i<MAX_THREADS; i++){
        threads[i].state = Unused;
    }
    threads[0].state = Main;

    interrupt_stack = malloc(STACK_SIZE);
    if(interrupt_stack == NULL){
        exit(1);
    }

    finish_stack = malloc(STACK_SIZE);
    if(finish_stack == NULL){
        exit(1);
    }

    gthread_setup_signal();
    gthread_setup_main();
    gthread_setup_finish();

    struct itimerval timer;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = NANOSECONDS * 1000;
    timer.it_value = timer.it_interval;

    setitimer(ITIMER_REAL, &timer, NULL);
}

void gthread_start(){
    setcontext(&threads[0].context);
}

void gthread_run(void *func){
	unsigned char *stack;
	gthread *thr;

	for (thr = &threads[0];; thr++){
		if (thr == &threads[MAX_THREADS]){
			exit(-1);
        }
		else if (thr->state == Unused){
			break;
        }
    }

	stack = malloc(STACK_SIZE);
	if (!stack){
	    exit(-1);
    }
    getcontext(&thr->context);

    thr->context.uc_stack.ss_sp = stack;
    thr->context.uc_stack.ss_size = STACK_SIZE;
    thr->context.uc_stack.ss_flags = 0;
    thr->context.uc_link = &finish_context;

    sigemptyset(&thr->context.uc_sigmask);

    makecontext(&thr->context, func, 0);
    thr->state = Ready;
}

void gthread_join(){
    int done = 1;
    do{
        done = 1;
        for(int i=0; i<MAX_THREADS; i++){
            if(threads[i].state != Unused){
                done = 0;
            }
        }
    }while(done != 0);
}

void a(){
    printf("A for the first time\n");
    for(int i=0;i< 1<<28;i++){

    }

    for(int i=0; i< 1<<20;i++){
        if(i == 1<<9){
            printf("a\n");
        }
    }
    printf("Done a\n");
}

void b(){
    for(int i=0; i< 1<<15;i++){
        if(i == 1<<10){
            printf("b\n");
        }
    }
    printf("Done b\n");
}

void c(){
    for(int i=0; i< 1<<10;i++){
        if(i==1<<3){
            printf("c\n");
        }
    }
    printf("Done c\n");
}

void d(){
    for(int i=0; i< 1<<10;i++){
        if(i==1<<7){
            printf("d\n");
        }
    }
    printf("Done d\n");
}

int main(void){
    gthread_init();

    gthread_run(a);
    gthread_run(b);
    gthread_run(c);
    gthread_run(d);

    gthread_start();
}
