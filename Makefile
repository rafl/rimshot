LIBS = -lasound -lusb
CFLAGS = -Wall -g
OBJECTS = $(patsubst %.c,%.o,$(wildcard *.c))

clean:
	$(RM) rimshot $(OBJECTS)

rimshot: $(OBJECTS)
	$(CC) -o $@ $^ $(LIBS)

all: rock
