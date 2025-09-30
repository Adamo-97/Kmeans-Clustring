# Compiler + flags
CC      = gcc
CSTD    = -std=c99
WARN    = -Wall -Wextra
OPT_R   = -O2
OPT_D   = -O0 -g            # debug: no optimizations + symbols
LDLIBS  =                   # add -lm later for your real code

# Files
SRC     = main.c kmeans.c
OBJ     = $(SRC:.c=.o)

# Default build: release
all: kmeans

# Release target (optimized)
CFLAGS_R = $(WARN) $(CSTD) $(OPT_R)
kmeans: $(OBJ)
	$(CC) $(CFLAGS_R) -o $@ $(OBJ) $(LDLIBS)

# Debug target (symbols, no opts)
CFLAGS_D = $(WARN) $(CSTD) $(OPT_D)
kmeans-debug: clean
	$(MAKE) CFLAGS="$(CFLAGS_D)" LDFLAGS="" build-debug

build-debug: $(OBJ)
	$(CC) $(CFLAGS) -o kmeans $^ $(LDLIBS)

# Pattern rule: object compilation picks up CFLAGS from context
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Convenience
run: kmeans
	./kmeans

valgrind: kmeans
	valgrind --leak-check=yes ./kmeans

clean:
	rm -f kmeans *.o

.PHONY: all kmeans-debug build-debug run clean
