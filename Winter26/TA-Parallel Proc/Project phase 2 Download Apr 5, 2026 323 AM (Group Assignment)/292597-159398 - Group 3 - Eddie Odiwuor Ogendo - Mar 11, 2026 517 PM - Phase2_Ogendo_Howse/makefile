# Compiler and flags
CC = mpicc
FLAGS = -Wall -Wextra -Wpedantic
LIBS = -lm
MPI = mpirun -np

# Target executable
TARGET = main
SRC_DIRS = . src
BUILD_DIR   := build
RELEASE_DIR := $(BUILD_DIR)/release

# Colours
GREEN  := \033[1;32m
BLUE   := \033[1;34m
YELLOW := \033[1;33m
RED    := \033[1;31m
RESET  := \033[0m

OUT_DIR   := $(RELEASE_DIR)
CFLAGS  := $(FLAGS) -fopenmp -O3 -march=native
BUILD_MSG := "Building in RELEASE mode"
LDFLAGS   := -fopenmp

SRCS := $(shell find $(SRC_DIRS) -maxdepth 1 -name "*.c" 2>/dev/null)
OBJS := $(SRCS:%.c=$(OUT_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

all: $(OUT_DIR)/$(TARGET)
	@echo -e "$(GREEN)Build Complete: $(OUT_DIR)/$(TARGET)$(RESET)"

$(OUT_DIR)/$(TARGET): $(OBJS)
	@echo -e "$(BLUE)$(BUILD_MSG)$(RESET)"
	@echo -e "$(GREEN)[Link]$(RESET) $@"
	$(CC) $(LDFLAGS) $(OBJS) -o $@ $(LIBS)

$(OUT_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo -e "$(YELLOW)[Compile]$(RESET) $<"
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

# mpirun -np processors main mode threads
# Run target
serial: $(OUT_DIR)/$(TARGET)
	@echo -e "$(GREEN)[Serial Run]$(RESET)"
	$(MPI) 1 ./$(OUT_DIR)/$(TARGET) 0 1

omp: $(OUT_DIR)/$(TARGET)
	@echo -e "$(GREEN)[OpenMP Run]$(RESET)"
	$(MPI) 1 ./$(OUT_DIR)/$(TARGET) 1 4

mpi: $(OUT_DIR)/$(TARGET)
	@echo -e "$(GREEN)[MPI Run]$(RESET)"
	$(MPI) 4 ./$(OUT_DIR)/$(TARGET) 2 1

leaks: $(OUT_DIR)/$(TARGET)
	@echo -e "$(BLUE)[Memcheck]$(RESET) Verifying cleanup."
	$(MPI) 1 valgrind --leak-check=full --show-leak-kinds=all -s ./$(OUT_DIR)/$(TARGET) 0 1

# Clean target
clean:
	@echo -e "$(RED)[Clean]$(RESET) Removing $(BUILD_DIR)"
	@rm -rf $(BUILD_DIR)


.PHONY: all clean serial omp mpi leaks
