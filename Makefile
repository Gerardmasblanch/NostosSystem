CC = gcc
CFLAGS = -Wall -Wextra

OBJECTES_COMUNS = utils.o config.o

all: odysseus ithaca island

odysseus: odysseus.o $(OBJECTES_COMUNS)
	$(CC) $(CFLAGS) -o odysseus odysseus.o $(OBJECTES_COMUNS)

ithaca: ithaca.o $(OBJECTES_COMUNS)
	$(CC) $(CFLAGS) -o ithaca ithaca.o $(OBJECTES_COMUNS)

island: island.o $(OBJECTES_COMUNS)
	$(CC) $(CFLAGS) -o island island.o $(OBJECTES_COMUNS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o odysseus ithaca island

.PHONY: all clean
