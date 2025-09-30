# Makefile for Kmeans project

# Compiler settings
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
LDFLAGS = -lm

# Target executables
TARGETS = kmeans main

# Default target - build all programs
all: $(TARGETS)

# Build kmeans executable
kmeans: kmeans.c
	$(CC) $(CFLAGS) -o kmeans kmeans.c $(LDFLAGS)

# Build main executable  
main: main.c
	$(CC) $(CFLAGS) -o main main.c $(LDFLAGS)

# Clean up compiled files
clean:
	rm -f $(TARGETS) *.o

# Run kmeans with sample data
run-kmeans: kmeans
	./kmeans kmeans-data.txt

# Run main program
run-main: main
	./main

# Help target
help:
	@echo "Available targets:"
	@echo "  all        - Build all programs (default)"
	@echo "  kmeans     - Build kmeans executable"
	@echo "  main       - Build main executable"
	@echo "  clean      - Remove compiled files"
	@echo "  run-kmeans - Build and run kmeans with data file"
	@echo "  run-main   - Build and run main program"
	@echo "  help       - Show this help message"

# Declare phony targets
.PHONY: all clean run-kmeans run-main help