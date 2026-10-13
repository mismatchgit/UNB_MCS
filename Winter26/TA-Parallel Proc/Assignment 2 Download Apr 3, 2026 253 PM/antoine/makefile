CC = gcc
MPICC = mpicc
CFLAGS = -O2 -lm

SRC = integration.c
SERIAL_EXE = serial
STATIC_EXE = static_mpi
DYNAMIC_EXE = dynamic_mpi

all: $(SERIAL_EXE) $(STATIC_EXE) $(DYNAMIC_EXE)

$(SERIAL_EXE): $(SRC)
	$(CC) $(CFLAGS) -DSERIAL_MODE -o $(SERIAL_EXE) $(SRC) $(LIBS)

$(STATIC_EXE): $(SRC)
	$(MPICC) $(CFLAGS) -o $(STATIC_EXE) $(SRC) $(LIBS)

$(DYNAMIC_EXE): $(SRC)
	$(MPICC) $(CFLAGS) -o $(DYNAMIC_EXE) $(SRC) $(LIBS)
