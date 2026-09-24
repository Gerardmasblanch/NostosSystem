CC = gcc
CFLAGS = -Wall -Wextra

all: odysseus ithaca island

odysseus: odysseus.o utils.o
	$(CC) $(CFLAGS) -o odysseus odysseus.o utils.o

ithaca: ithaca.o utils.o
	$(CC) $(CFLAGS) -o ithaca ithaca.o utils.o

island: island.o utils.o
	$(CC) $(CFLAGS) -o island island.o utils.o

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o odysseus ithaca island

.PHONY: all clean
