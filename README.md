# The Nostos System

Practica de Sistemes Operatius, curs 2026-2027.

Autors: Arnau Ricart (arnau.ricart) i Gerard Mas (gerard.mb)

## Compilacio

```bash
make
```

Genera els tres executables: `odysseus`, `ithaca` i `island`.

```bash
make clean
```

## Execucio

```bash
./odysseus <config.dat>
./ithaca <config.dat> <voyages.dat>
./island <config.dat> <stock.db>
```

## Entrega

```bash
make clean
tar cf Gx_F1.tar *.c *.h Makefile *.dat *.db
```

## Calendari

| Fase | Deadline |
|---|---|
| Fase 1 | 04/10/2026 |
| Fase 2 | 01/11/2026 |
| Lliurament Final 1 | 22/11/2026 |
| Lliurament Final 2 | 06/12/2026 |
| Lliurament Final 3 | 10/01/2027 |

## Notes

- Tot el codi es compila amb `-Wall -Wextra`; cap warning.
- Nomes `read`/`write` per a entrada/sortida (`asprintf` permes).
- Prohibides `printf`, `scanf`, `gets`, `puts`, `system`, `popen`, `stat`.
- La compilacio i les proves valides son les de **Montserrat**, no les locals.
