CC = gcc
MPICC = mpicc
CFLAGS = -O3 -lm

SRC = integration.c
SERIAL_EXE = serial
MPI_EXE = mpi
MPI_HYBRID_EXE = mpi_hybrid

all: $(SERIAL_EXE) $(MPI_HYBRID_EXE) $(MPI_EXE)

$(SERIAL_EXE): $(SRC)
	$(CC) $(CFLAGS) -DSERIAL_MODE -o $(SERIAL_EXE) $(SRC)

$(MPI_EXE): $(SRC)
	$(MPICC) $(CFLAGS) -o $(MPI_EXE) $(SRC)

$(MPI_HYBRID_EXE): $(SRC)
	$(MPICC) $(CFLAGS) -DHYBRID -fopenmp -o $(MPI_HYBRID_EXE) $(SRC)
