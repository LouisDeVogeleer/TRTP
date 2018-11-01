GCC = gcc
# See gcc/clang manual to understand all flags
CFLAGS += -std=gnu99  # Define which version of the C standard to use
CFLAGS += -Wall # Enable the 'all' set of warnings
CFLAGS += -Werror # Treat all warnings as error
CFLAGS += -Wshadow # Warn when shadowing variables
CFLAGS += -Wextra # Enable additional warnings
CFLAGS += -lm #getopt
CFLAGS += -O2 -D_FORTIFY_SOURCE=2 # Add canary code, i.e. detect buffer overflows
CFLAGS += -fstack-protector-all # Add canary code to detect stack smashing
CFLAGS += -g

# We have no libraries to link against except libc, but we want to keep
# the symbols for debugging
LDFLAGS = -lz
LDFLAGS += -rdynamic

# Default target
all: receiver sender

sender: src/sender.o src/socket.o src/queue.o src/packet_implem.c

receiver: src/receiver.o src/socket.o src/queue.o src/packet_implem.c

%: src/%.o
	$(CC) $^ -o $@ $(LDFLAGS)

clean :
		rm src/*.o
		rm sender
		rm receiver
# We use an implicit rule: look for the files record.c/database.c,
# compile them and link the resulting *.o's into an executable named database

.PHONY: clean
