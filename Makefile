CC = gcc

# _GNU_SOURCE ha d'estar definida abans de qualsevol capcalera del sistema.
# Definir-la aqui evita que depengui de l'ordre dels includes de cada fitxer.
# Inclou ja _XOPEN_SOURCE i _POSIX_C_SOURCE, no cal definir-les a part.
CFLAGS = -Wall -Wextra -D_GNU_SOURCE

# sphragis.o ve compilat des de l'eStudy (objecte ELF de Linux: nomes enllaca
# a Montserrat, no a Windows). Nomes depen de la libc, no cal cap -l extra.
OBJECTES_COMUNS = utils.o config.o sphragis.o

all: odysseus ithaca island

odysseus: odysseus.o $(OBJECTES_COMUNS)
	$(CC) $(CFLAGS) -o odysseus odysseus.o $(OBJECTES_COMUNS)

ithaca: ithaca.o $(OBJECTES_COMUNS)
	$(CC) $(CFLAGS) -o ithaca ithaca.o $(OBJECTES_COMUNS)

island: island.o $(OBJECTES_COMUNS)
	$(CC) $(CFLAGS) -o island island.o $(OBJECTES_COMUNS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

# sphragis.o no es compila: ja ve fet. Per aixo clean no l'esborra.
clean:
	rm -f utils.o config.o odysseus.o ithaca.o island.o odysseus ithaca island

.PHONY: all clean
