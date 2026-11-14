CC = gcc

# _GNU_SOURCE ha d'estar definida abans de qualsevol capcalera del sistema.
# Definir-la aqui evita que depengui de l'ordre dels includes de cada fitxer.
# Inclou ja _XOPEN_SOURCE i _POSIX_C_SOURCE, no cal definir-les a part.
# Els -I permeten fer #include "trama.h" des de qualsevol carpeta de src.
CFLAGS = -Wall -Wextra -D_GNU_SOURCE -Isrc/common -Isrc/odysseus -Isrc/ithaca -Isrc/island

COMMON   = src/common
ODYSSEUS = src/odysseus
ITHACA   = src/ithaca
ISLAND   = src/island

# sphragis.o ve compilat des de l'eStudy (objecte ELF de Linux: nomes enllaca
# a Montserrat, no a Windows). Nomes depen de la libc, no cal cap -l extra.
OBJECTES_COMUNS = $(COMMON)/utils.o $(COMMON)/config.o $(COMMON)/trama.o $(COMMON)/sphragis.o

OBJECTES_ODYSSEUS = $(ODYSSEUS)/odysseus.o $(ODYSSEUS)/commands.o $(ODYSSEUS)/navegant.o $(ODYSSEUS)/travessia.o
OBJECTES_ITHACA   = $(ITHACA)/ithaca.o $(ITHACA)/voyages.o
OBJECTES_ISLAND   = $(ISLAND)/island.o $(ISLAND)/harbor.o $(ISLAND)/market.o

all: odysseus ithaca island

odysseus: $(OBJECTES_ODYSSEUS) $(OBJECTES_COMUNS)
	$(CC) $(CFLAGS) -o odysseus $(OBJECTES_ODYSSEUS) $(OBJECTES_COMUNS)

ithaca: $(OBJECTES_ITHACA) $(OBJECTES_COMUNS)
	$(CC) $(CFLAGS) -o ithaca $(OBJECTES_ITHACA) $(OBJECTES_COMUNS) -lpthread

island: $(OBJECTES_ISLAND) $(OBJECTES_COMUNS)
	$(CC) $(CFLAGS) -o island $(OBJECTES_ISLAND) $(OBJECTES_COMUNS)

# Cada .o depen de tots els .h: si es canvia una capcalera (un #define,
# un struct...) es recompila tot el que la podria fer servir.
CAPCALERES = $(COMMON)/types.h $(COMMON)/utils.h $(COMMON)/config.h $(COMMON)/trama.h $(COMMON)/sphragis.h \
             $(ODYSSEUS)/commands.h $(ODYSSEUS)/navegant.h $(ODYSSEUS)/travessia.h \
             $(ITHACA)/voyages.h \
             $(ISLAND)/harbor.h $(ISLAND)/market.h

%.o: %.c $(CAPCALERES)
	$(CC) $(CFLAGS) -c $< -o $@

# sphragis.o no es compila: ja ve fet. Per aixo clean no l'esborra.
clean:
	rm -f $(COMMON)/utils.o $(COMMON)/config.o $(COMMON)/trama.o $(OBJECTES_ODYSSEUS) $(OBJECTES_ITHACA) $(OBJECTES_ISLAND) odysseus ithaca island

.PHONY: all clean
