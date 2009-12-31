LIBS = -lasound -lusb
CFLAGS = -Wall -g
OBJECTS = $(patsubst %.c,%.o,$(wildcard *.c))

all: rimshot

clean:
	$(RM) rimshot $(OBJECTS)

rimshot: $(OBJECTS)
	$(CC) -o $@ $^ $(LIBS)
