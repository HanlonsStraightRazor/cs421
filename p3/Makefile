CC = gcc
CFLAGS = -Wall -pthread -g
OBJECTS = p3.o

p3 : $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $?

.PHONY : clean
clean :
	rm -f p3 $(OBJECTS)
