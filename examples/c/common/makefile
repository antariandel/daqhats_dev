CC=gcc
CFLAGS = -I/usr/local/include
OFLAGS = -ldaqhats
TARGETS = list_boards read_values

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

all: $(TARGETS)

list_boards: list_boards.o
	$(CC) -o $@ $^ $(OFLAGS)

read_values: read_values.o
	$(CC) -o $@ $^ $(OFLAGS)

.PHONY: clean

clean:
	rm -f *.o *~ $(TARGETS)
