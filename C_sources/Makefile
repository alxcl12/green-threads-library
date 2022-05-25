CC=gcc

all:
	$(CC) -Wall -o copy_files ../gthreads.c copy_files.c
	$(CC) -Wall -o image_filter ../gthreads.c image_filter.c
	$(CC) -Wall -o make_blocks ../gthreads.c make_blocks.c -lm

clean:
	rm -rf copy_files
	rm -rf image_filter
	rm -rf make_blocks
	rm -rf out/blocks/*
	rm -rf out/image/*
	rm -rf out/files/*