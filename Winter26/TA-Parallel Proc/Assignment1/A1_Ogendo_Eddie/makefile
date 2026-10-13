# Compiler and flags
CC = gcc
CFLAGS = -fopenmp -Wall -O3
LIBS = -lm

# Target executable
TARGET = Asn1
SRC = Asn1.c

# Matric dimension
N = 512

# Default target
all: $(TARGET)

# Build target
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LIBS) -o $(TARGET)

# Run target
run0: $(TARGET)
	./$(TARGET) $(N) 0

run1: $(TARGET)
	./$(TARGET) $(N) 1

run2: $(TARGET)
	./$(TARGET) $(N) 2

run3: $(TARGET)
	./$(TARGET) $(N) 3

run4: $(TARGET)
	./$(TARGET) $(N) 4
	
run5: $(TARGET)
	./$(TARGET) $(N) 5

run6: $(TARGET)
	./$(TARGET) $(N) 6

memcheck: $(TARGET)
	@echo -e "Verifying RAII cleanup."
	@valgrind --leak-check=full --show-leak-kinds=all -s ./$(TARGET) $(N) 5

# Clean target
clean:
	rm -f $(TARGET)
