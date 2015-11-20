CC=gcc
CFLAGS = -Wall -Wswitch-default -Wextra -Wunreachable-code -Wshadow -Wpointer-arith -Wmissing-declarations -Wredundant-decls -Wstrict-aliasing=2 -Wdouble-promotion -Wcast-align -Wwrite-strings
SOURCE_FILES = main.c handlers.c pinpoint_utility_functions.c
HEADER_FILES = handlers.h unpack_macros.h socket_includes.h pinpoint_utility_functions.h
TARGET = mab_pinpoint_interface

ifeq ($(OS),Windows_NT)
TARGET := $(TARGET).exe
CFLAGS += -lws2_32
endif

$(TARGET): makefile $(SOURCE_FILES) $(HEADER_FILES)
	$(CC) $(SOURCE_FILES) $(CFLAGS) -o $@

clean:
	rm -rf $(TARGET) $(TARGET).exe
