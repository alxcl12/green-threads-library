#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include "gthreads.h"

typedef struct
{
    int* r;
    int* g;
    int* b;
} RGBImage;

RGBImage rgbImage;

#define WIDTH 800
#define HEIGHT 600

int ReadHeader(FILE* filePointer)
{
    int width, height, bitMax, fileMode;
    char ch;
    if (fscanf(filePointer, "P%c\n", &ch) != 1)
    {
        //bad read
        return -1;
    }

    if (ch == '3')
    {
        fileMode = 3;
    }

    if (ch == '6')
    {
        fileMode = 6;
    }

    //skipping comment
    ch = getc(filePointer);
    while (ch == '#')
    {
        do
        {
            ch = getc(filePointer);
        } while (ch != '\n');
        ch = getc(filePointer);
    }

    //asuming 1 line comment, next up should be resolution
    if (!isdigit(ch))
    {
        //bad format
        return -1;
    }

    ungetc(ch, filePointer); //put back first digit of resolution

    fscanf(filePointer, "%d%d%d\n", &width, &height, &bitMax);

    if (bitMax != 255)
    {
        //bad color mode
        return -1;
    }

    return fileMode;
}

void ReadPicture(const char* filename)
{
    FILE* filePointer;
    filePointer = fopen(filename, "rb");

    if (filePointer)
    {
        if (ReadHeader(filePointer) != 6)
        {
            //bad image header
            return;
        }

        uint8_t r, g, b;
        char newLine;

        for (int i = 0; i < HEIGHT; i++)
        {
            for (int j = 0; j < WIDTH; j++)
            {
                fread(&r, sizeof(r), 1, filePointer);
                //fread(&newLine, sizeof(newLine), 1, filePointer);

                fread(&g, sizeof(g), 1, filePointer);
                //fread(&newLine, sizeof(newLine), 1, filePointer);

                fread(&b, sizeof(b), 1, filePointer);
                //fread(&newLine, sizeof(newLine), 1, filePointer);

                rgbImage.r[(i * WIDTH) + j] = r;
                rgbImage.g[(i * WIDTH) + j] = g;
                rgbImage.b[(i * WIDTH) + j] = b;
            }
        }
    }

    fclose(filePointer);
}

void filter_image(int posIBlock, int posJBlock, int empty)
{
    // int posIBlock = blockIdx.x;
    // int posJBlock = blockIdx.y;

    //printf("I: %d   J: %d\n", posIBlock, posJBlock);
    printf("i: %d; j: %d\n", posIBlock, posJBlock);
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            float nR, nG, nB;
            int pos = ((i + posIBlock) * WIDTH) + j + posJBlock;
            nR = (float)rgbImage.r[pos] * 0.39 + (float)rgbImage.g[pos] * 0.75 + (float)rgbImage.b[pos] * 0.19;
            nG = (float)rgbImage.r[pos] * 0.35 + (float)rgbImage.g[pos] * 0.69 + (float)rgbImage.b[pos] * 0.17;
            nB = (float)rgbImage.r[pos] * 0.27 + (float)rgbImage.g[pos] * 0.53 + (float)rgbImage.b[pos] * 0.13;

            //printf("Pixel pos: %d\n R before: %d\n R after: %d\n", pos, rgbImage.r[pos], (int)nR);

            rgbImage.r[pos] = (int)nR;
            rgbImage.g[pos] = (int)nG;
            rgbImage.b[pos] = (int)nB;

            if (rgbImage.r[pos] > 255) {
                rgbImage.r[pos] = 255;
            }
            if (rgbImage.g[pos] > 255) {
                rgbImage.g[pos] = 255;
            }
            if (rgbImage.b[pos] > 255) {
                rgbImage.b[pos] = 255;
            }
        }
    }
}

int main(){
    rgbImage.r = (int*)malloc(WIDTH * HEIGHT * sizeof(int));
    rgbImage.g = (int*)malloc(WIDTH * HEIGHT * sizeof(int));
    rgbImage.b = (int*)malloc(WIDTH * HEIGHT * sizeof(int));

    ReadPicture("nt-P6.ppm");

    gthread_init();

    int threads[7500];
    int counter = 0;
    for(int i = 0; i<HEIGHT; i+=8){
        for(int j = 0; j<WIDTH; j+=8){
            //filter
            threads[counter] = gthread_run(filter_image,i,j,0);
            //printf("%d\n", threads[counter]);
            counter++;
        }
    }
    for(int i = 0;i<counter;i++){
        gthread_join(threads[i]);
    }


    //write to file
    FILE* test;
    test = fopen("test.ppm", "wb");
    fprintf(test, "P6\n# CREATOR: GIMP PNM Filter Version 1.1\n800 600\n255\n");
    for (int i = 0; i < HEIGHT; i++)
    {
        for (int j = 0; j < WIDTH; j++)
        {
            fwrite(&rgbImage.r[(i * WIDTH) + j], 1, 1, test);
            fwrite(&rgbImage.g[(i * WIDTH) + j], 1, 1, test);
            fwrite(&rgbImage.b[(i * WIDTH) + j], 1, 1, test);
        }
    }
    fclose(test);


    free(rgbImage.r);
    free(rgbImage.g);
    free(rgbImage.b);
    return 0;
}