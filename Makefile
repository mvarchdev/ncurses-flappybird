# variables
CC=gcc
CFLAGS=-std=c11 -Wall -Werror -Wno-unused-parameter
LDLIBS=-lcurses
OUTPUT=flappy_bird

# targets
all: $(OUTPUT)

$(OUTPUT): clean
	$(CC) $(CFLAGS) *.c $(LDLIBS) -o $(OUTPUT)

# remove compiled files
clean:
	rm -rf $(OUTPUT) *.o
