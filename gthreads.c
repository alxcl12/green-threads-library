#include <stdio.h>
#include <stdlib.h>
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

gthread threads[MAX_THREADS];
gthread *thread_current;
int thread_currrent_id = 0;

ucontext_t interrupt_context;
unsigned char *interrupt_stack;
sigset_t interrupt_sigmask;

ucontext_t finish_context;
unsigned char *finish_stack;

void gthread_schedule(){
    //printf("On interrupt, thread ID: %d\n", thread_currrent_id);

    do{
        thread_currrent_id = (thread_currrent_id + 1) % MAX_THREADS;
    }while(threads[thread_currrent_id].state != Ready);

    thread_current = &threads[thread_currrent_id];

    //printf("On interrupt exit, thread ID to be run: %d\n", thread_currrent_id);
    thread_current->state = Running;
    setcontext(&thread_current->context);
}

void gthread_timer_interrupt_handler(int warn, siginfo_t *sig, void *prev_context){
    printf("On handler..\n");
    getcontext(&interrupt_context);
    interrupt_context.uc_stack.ss_sp = interrupt_stack;
    interrupt_context.uc_stack.ss_size = STACK_SIZE;

    sigemptyset(&interrupt_context.uc_sigmask);
    makecontext(&interrupt_context, gthread_schedule, 1);

    swapcontext(&thread_current->context, &interrupt_context);
}

void gthread_setup_signal(){
    struct sigaction action;

    action.sa_sigaction = gthread_timer_interrupt_handler;
    sigemptyset(&action.sa_mask);

    sigemptyset(&interrupt_sigmask);
    sigaddset(&interrupt_sigmask, SIGALRM);

    sigaction(SIGALRM, &action, NULL);
}

void gthread_main_runner(){
    gthread_schedule();
}

void gthread_finish_runner(){
    threads[thread_currrent_id].state = Finished;
    gthread_schedule();
}

void gthread_setup_main(){
	unsigned char *stack;
	gthread *thr = &threads[0];

	stack = malloc(STACK_SIZE);
	if (!stack){
	    exit(-1);
    }
    getcontext(&thr->context);

    thr->context.uc_stack.ss_sp = stack;
    thr->context.uc_stack.ss_size = STACK_SIZE;
    thr->context.uc_stack.ss_flags = 0;

    sigemptyset(&thr->context.uc_sigmask);

    makecontext(&thr->context, gthread_main_runner, 1);
    thr->state = Main;
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

    makecontext(&finish_context, gthread_finish_runner, 1);
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

    makecontext(&thr->context, func, 1);
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
    printf("IN A");
    for(int i=0;i< 1<<30;i++){
        for(int j=0;j< 1<<30;j++){

        }
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

    // //jump start
    // thread_current = &threads[0];
    // setcontext(&threads[0].context);

    //gthread_join();
    gthread_start();
}
