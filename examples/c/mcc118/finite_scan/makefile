OBJ = finite_scan.o
LIBS = -ldaqhats
CFLAGS = -Wall -I/usr/local/include -g
CC = gcc
EXTENSION = .c

all: finite_scan

%.o: %$(EXTENSION) $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

finite_scan: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f *.o *~ core *~ 
