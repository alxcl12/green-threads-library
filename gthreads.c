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
    int waiting_on;
} gthread;

typedef struct gthr_mutex {
    int owner;
	enum {
        Unlocked,
        Locked
	} state;
} gthread_mutex;

gthread_mutex mut;

gthread threads[MAX_THREADS];
gthread *thread_current;
int thread_currrent_id = 0;

ucontext_t interrupt_context;
unsigned char *interrupt_stack;
sigset_t interrupt_sigmask;

ucontext_t finish_contexts[MAX_THREADS];

int running_threads = 0;
struct itimerval timer;

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

void gthread_timer_interrupt_handler(int warn, siginfo_t *sig, void *prev_context){
    #ifdef DEBUG
    printf("On handler..\n");
    #endif
    // printf("NIEZ\n");
    if(thread_current->state != Finished && thread_current->state != Main){
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

void pause_timer()
{
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;
    timer.it_value = timer.it_interval;
    setitimer(ITIMER_REAL, &timer, 0);
}

void resume_timer()
{
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = NANOSECONDS * 1000;
    timer.it_value = timer.it_interval;
    setitimer(ITIMER_REAL, &timer, NULL);
}

void gthread_finish_runner(){
    threads[thread_currrent_id].state = Finished;
    running_threads--;
    printf("In finish:%d\n", threads[0]);
    #ifdef DEBUG
    printf("COUNTER: %d\n", cn);
    #endif

    // getcontext(&threads[0].context);
    // // threads[0].context.uc_stack.ss_sp = threads[0].context.;
    // // threads[0].context.uc_stack.ss_size = STACK_SIZE;

    // // sigemptyset(&threads[0].context.uc_sigmask);
    // makecontext(&threads[0].context, gthread_main_runner, 0);

    // thread_currrent_id = 0;
    // swapcontext(&thread_current->context, &threads[0].context);
}

void gthread_init(void){
    for(int i=0; i<MAX_THREADS; i++){
        threads[i].state = Unused;
        threads[i].waiting_on = -1;
    }
    threads[0].state = Main;

    interrupt_stack = malloc(STACK_SIZE);
    if(interrupt_stack == NULL){
        exit(1);
    }

    gthread_setup_signal();
}


gthread gthread_run(void *func){
    unsigned char *stack, *finish_stack;
    gthread *thr;

    int i=0;
	for (i = 0;i < MAX_THREADS; i++){
		if (i == MAX_THREADS){
			exit(-1);
        }
		else if (threads[i].state == Unused){
			thr = &threads[i];
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
    thr->context.uc_link = &finish_contexts[i];

    sigemptyset(&thr->context.uc_sigmask);

    makecontext(&thr->context, func, 0);
    thr->state = Ready;

    /////////////////////////////////////
    finish_stack = malloc(STACK_SIZE);
	if (!finish_stack){
	    exit(-1);
    }
    getcontext(&finish_contexts[i]);

    finish_contexts[i].uc_stack.ss_sp = finish_stack;
    finish_contexts[i].uc_stack.ss_size = STACK_SIZE;
    finish_contexts[i].uc_stack.ss_flags = 0;
    finish_contexts[i].uc_link = &threads[0].context;

    sigemptyset(&finish_contexts[i].uc_sigmask);

    makecontext(&finish_contexts[i], gthread_finish_runner, 0);

    if (running_threads == 0){
        getcontext(&threads[0].context);

        thread_current = &threads[0];

        
        timer.it_interval.tv_sec = 0;
        timer.it_interval.tv_usec = NANOSECONDS * 1000;
        timer.it_value = timer.it_interval;

        setitimer(ITIMER_REAL, &timer, NULL);
    }
    running_threads++;
    return *thr;
    //create main
    //get context only for main
}

void gthread_join(gthread thread){
    if (thread.waiting_on == -1 && thread.state != Finished){
        pause_timer();
        swapcontext(&threads[0].context, &thread.context);
        resume_timer();
    }
}

void gthread_mutex_init(gthread_mutex *mutex){
    mutex->owner = -1;
    mutex->state = Unlocked;
}

void gthread_mutex_lock(gthread_mutex *mutex){
    if(mutex->state == Unlocked){
        mutex->owner = thread_currrent_id;
        mutex->state = Locked;
        printf("In lock unlocked: %d %d\n", mutex->owner, thread_currrent_id);
    }
    else{
        //schedule owner
        printf("In lock lock: %d\n", mutex->owner);
        getcontext(&interrupt_context);
        interrupt_context.uc_stack.ss_sp = interrupt_stack;
        interrupt_context.uc_stack.ss_size = STACK_SIZE;

        sigemptyset(&interrupt_context.uc_sigmask);
        makecontext(&interrupt_context, gthread_schedule, 1, mutex->owner);

        swapcontext(&thread_current->context, &interrupt_context);
    }
}

void gthread_mutex_unlock(gthread_mutex *mutex){
    if(mutex->state == Locked && mutex->owner == thread_currrent_id){
        mutex->owner = -1;
        mutex->state = Unlocked;
    }
}

void a(){
    printf("A for the first time\n");
    //gthread_mutex_lock(&mut);
    for(int i=0;i< 1<<28;i++){

    }

    for(int i=0; i< 1<<20;i++){
        if(i == 1<<9){
            //gthread_mutex_unlock(&mut);
            printf("a\n");
        }
    }
    printf("Done a\n");
}

void b(){
    //gthread_mutex_lock(&mut);
    //gthread_mutex_unlock(&mut);
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
    gthread_mutex_init(&mut);
    gthread p1,p2,p3,p4;
    gthread_init();

    p1=gthread_run(a);
    p2=gthread_run(b);
    p3=gthread_run(c);
    p4=gthread_run(d);


    // for(int i = 0; i < 1<<28;i++){
    //     if(i==1<<27){
    //         printf("Hey\n");
    //     }
    // }
    //     for(int i = 0; i < 1<<28;i++){
    //     if(i==1<<27){
    //         printf("Hey2\n");
    //     }
    // }
    gthread_join(p4);
    gthread_join(p3);
    gthread_join(p2);
    gthread_join(p1);


    int x;
}
