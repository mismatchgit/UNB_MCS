CC=mpicc
CFLAGS=-O3 -fopenmp
LDLIBS=-lm

all: integration

integration: integration.c
	$(CC) $(CFLAGS) integration.c -o integration $(LDLIBS)

clean:
	rm -f integration
