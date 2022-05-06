#include "gthreads.h"

int a(int a,int b,int c){
    printf("%d %d %d\n",a,b,c);
}


int b(int a,int b,int c){
    printf("%d %d %d\n",a,b,c);
}


int c(int a,int b,int c){
    printf("%d %d %d\n",a,b,c);
}

int main(){
    gthread_init();
    int g,h,j;
    g=gthread_run(a,1,2,3);
    h=gthread_run(b,4,5,6);
    j=gthread_run(c,7,8,9);

    gthread_join(g);
    gthread_join(h);
    gthread_join(j);
    return 0;
}