#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "gthreads.h"

#define WIDTH 800
#define HEIGHT 600

typedef struct position_str{
    int i;
    int j;
}position;

int matrix[HEIGHT][WIDTH];

char* itoa(int val, int base){
	static char buf[32] = {0};

	int i = 30;

	for(; val && i ; --i, val /= base)
		buf[i] = "0123456789abcdef"[val % base];

	return &buf[i+1];
}

void read_matrix(void){
    FILE* file;
    file = fopen("../input/matrix/matrix.in", "r");

    for (int i = 0; i < HEIGHT; i++){
        for (int j = 0; j < WIDTH; j++){
            fscanf(file, "%d", &matrix[i][j]);
        }
    }
    fclose(file);

}

void shuffle(position *array, int n)
{
    if (n > 1){
        int i;
        for (i = 0; i < n - 1; i++){
          int j = i + rand() / (RAND_MAX / (n - i) + 1);
          position t = array[j];
          array[j] = array[i];
          array[i] = t;
        }
    }
}

void write_block(int posIBlock, int posJBlock, int index)
{
    FILE *out;

    char filename_out[19] = "out/blocks/block";
    char* nr;
    nr = itoa(index,10);

    int digits = index / 10 + 1;

    strncat(filename_out, nr, digits);
    //printf("%s\n",filename_out);

    out = fopen(filename_out, "w");
    double polynom = 0;
    for (int i = 0; i < 8; i++){
        for (int j = 0; j < 8; j++){
            fprintf(out, "%d ", matrix[posIBlock+i][posJBlock+j]);
            if(i == j){
                polynom += pow((double)matrix[posIBlock+i][posJBlock+j], (double)i);
            }
        }
        fprintf(out,"\n");
    }
    fprintf(out, "%lf", polynom);
    fclose(out);
}

int main(){
    read_matrix();
    gthread_init();

    int threads[7500];
    position pos[7500];
    int counterPos = 0;
    int counter = 0;

    for(int i = 0; i<HEIGHT; i+=8){
        for(int j = 0; j<WIDTH; j+=8){
            pos[counterPos].i = i;
            pos[counterPos].j = j;
            counterPos++;
        }
    }

    shuffle(pos, 7500);

    struct timeval start,stop;
    gettimeofday(&start, NULL);
    long long startTime = (long long)start.tv_sec * 1000 + (long long)start.tv_usec / 1000;

    for(int i=0; i<7500; i++){
        threads[counter] = gthread_run(write_block,pos[i].i,pos[i].j,i);
        counter++;
    }

    for(int i = 0;i<counter;i++){
        gthread_join(threads[i]);
    }

    gettimeofday(&stop, NULL);
    long long endTime = (long long)stop.tv_sec * 1000 + (long long)stop.tv_usec / 1000;
    long long time = endTime - startTime;

    printf("%lld\n", time);

    return 0;
}