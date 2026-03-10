# Compiler and Flags
CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude
SRC_DIR = src
SRCS = $(wildcard $(SRC_DIR)/*.c)

# Target: all
# Description: Compile the simulator
all: schedsim

# Rule to build the executable
schedsim: $(SRCS)
	$(CC) $(CFLAGS) -o schedsim $(SRCS)

# Target: clean
# Description: Remove binaries and object files
clean:
	rm -f schedsim *.o

# helper to prevent conflicts with files named 'all' or 'clean'
.PHONY: all clean