CC=gcc

gthreads: gthreads.c
	$(CC) -o gthreads gthreads.c

gthreads_debug: gthreads.c
	$(CC) -o gthreads_debug gthreads.c -DDEBUG