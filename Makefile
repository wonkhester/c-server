# Compiler to use
CC = gcc

# Common compiler flags
CFLAGS = -Wall -g

# Platform-specific flags
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S), Darwin)
    # macOS-specific flags
    CFLAGS += -arch x86_64 -arch arm64
endif

# List of source files to compile
SRC = server.c router.c router_manager.c file_manager.c

# Output executable name
OUT = web_server

# Files to be formatted
FORMAT_SRC = $(SRC) router.h router_manager.h file_manager.h

# Default target (format code, then build)
all: format $(OUT)

# How to build the executable
$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

# Target to format the C files
format:
	clang-format -i $(FORMAT_SRC)

# Clean up compiled files
clean:
	rm -f $(OUT)
