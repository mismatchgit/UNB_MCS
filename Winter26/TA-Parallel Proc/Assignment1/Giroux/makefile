CC = gcc
CFLAGS = -fopenmp -lm -O3

all: assignment

# this is used to define additional arguments for compile time
ARG=

assignment: a1.c
ifdef ARG
	$(CC) $(CFLAGS) -o a1 a1.c
else
	$(CC) $(CFLAGS) -D ARG -o a1 a1.c
endif

# redefine the size of the matrix by writing N=<number> when running the command for make run
N = 512
# the mode is 1 by default, you can redefine it by writing MODE=<number> when inputting the command for make run
MODE = 0

THREAD_COUNT =

ITER = 1


run: assignment
	for ((i=0; i<$(ITER); i++)); do \
		./a1 $(N) $(MODE) $(THREAD_COUNT); \
	done

clean:
	rm -f ./a1



