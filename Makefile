LIBS = -lasound -lusb
CFLAGS = -Wall -g
OBJECTS = $(patsubst %.c,%.o,$(wildcard *.c))

rimshot: $(OBJECTS)
	$(CC) -o $@ $^ $(LIBS)

all: rock
