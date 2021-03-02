CC = gcc
CFLAGS = -Wall -g -pipe
OBJECTS = p4.o queue.o

p4: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $?

.PHONY: clean
clean:
	rm -rf p4 $(OBJECTS)
