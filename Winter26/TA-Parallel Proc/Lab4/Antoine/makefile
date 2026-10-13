CC = mpicc
CFLAGS = -O3

FILE = lab4.c
BIN = lab4

MSG_SIZE ?= 1

all: $(BIN)

$(BIN): $(FILE)
	$(CC) $(CFLAGS) -DMSG_SIZE=$(MSG_SIZE) -o $(BIN) $(FILE)

clean:
	rm -f $(BIN)

pingpong: $(BIN)
	mpirun --bind-to-core -np 2 ./$(BIN) pingpong
	$(MAKE) clean

P ?= 2

ring: $(BIN)
	mpirun --bind-to-core -np $(P) ./$(BIN) ring
	$(MAKE) clean
