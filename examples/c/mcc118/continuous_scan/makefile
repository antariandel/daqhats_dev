OBJ = continuous_scan.o
LIBS = -ldaqhats
CFLAGS = -Wall -I/usr/local/include -g
CC = gcc
EXTENSION = .c

all: continuous_scan

%.o: %$(EXTENSION) $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

continuous_scan: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f *.o *~ core *~ 
