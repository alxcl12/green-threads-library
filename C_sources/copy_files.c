#include "gthreads.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

char* itoa(int val, int base){
	static char buf[32] = {0};

	int i = 30;

	for(; val && i ; --i, val /= base)
		buf[i] = "0123456789abcdef"[val % base];

	return &buf[i+1];
}

void copy(int arg1, int arg2, int arg3){
    FILE *in, *out;
    char buffer[1024];

    char filename_in[16] = "../../files/tst";
    char* nr;
    nr = itoa(arg1,10);

    int digits = 1;
    if(arg1 > 9){
        digits = 2;
    }

    strncat(filename_in, nr, digits);

    in = fopen(filename_in, "rb");

    filename_in[12] = 'o';
    out = fopen(filename_in, "w");

    if(in && out){
        while(fread(&buffer, sizeof(char), 1024, in) != 0){
            fwrite(&buffer, sizeof(char), 1024, out);
        }
    }
    fclose(in);
    fclose(out);
}

int main(){
    int threads[101];
    gthread_init();
    int counter = 1;

    struct timeval start,stop;
    gettimeofday(&start, NULL);
    long long startTime = (long long)start.tv_sec * 1000 + (long long)start.tv_usec / 1000;

    for(int i=1;i<101;i++){
        threads[counter]=gthread_run(copy,i,0,0);
        counter++;
    }

    for(int i=1;i<101;i++){
        gthread_join(threads[i]);
    }

    gettimeofday(&stop, NULL);
    long long endTime = (long long)stop.tv_sec * 1000 + (long long)stop.tv_usec / 1000;
    long long time = endTime - startTime;

    printf("%lld\n", time);

    return 0;
}