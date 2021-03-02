CC = gcc
CFLAGS = -Wall -g
OBJECTS = mash.o history.o commandlist.o

mash : $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $?

history.o : history.h
commandlist.o : commandlist.h

.PHONY : clean
clean :
	rm -f mash $(OBJECTS)
