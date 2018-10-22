GCC = gcc
# See gcc/clang manual to understand all flags
CFLAGS += -std=c99 # Define which version of the C standard to use
CFLAGS += -Wall # Enable the 'all' set of warnings
CFLAGS += -Werror # Treat all warnings as error
CFLAGS += -Wshadow # Warn when shadowing variables
CFLAGS += -Wextra # Enable additional warnings
CFLAGS += -lm #getopt
CFLAGS += -O2 -D_FORTIFY_SOURCE=2 # Add canary code, i.e. detect buffer overflows
CFLAGS += -fstack-protector-all # Add canary code to detect stack smashing

# We have no libraries to link against except libc, but we want to keep
# the symbols for debugging
LDFLAGS= -lz
LDFLAGS= -rdynamic

# Default target
all: sender receiver 

sender: src/sender.o src/socket.o src/packet_implement.c

receiver: src/receiver.o src/socket.o src/packet_implement.c

%: src/%.o
	$(CC) $^ -o $@ $(LDFLAGS)

# We use an implicit rule: look for the files record.c/database.c,
# compile them and link the resulting *.o's into an executable named database

.PHONY: clean
//TODO
