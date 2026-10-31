THE NOSTOS SYSTEM - FASE 2
Grup 1: Arnau Ricart (arnau.ricart) i Gerard Mas (gerard.mb)


COMPILAR

    make
    make clean


EXECUTAR

    puigpedros   ./ithaca config/ithaca.dat config/voyages.dat

    matagalls    ./island config/aeaea.dat stock/Aeaea.db
                 ./island config/aeolia.dat stock/Aeolia.db
                 ./island config/thrinacia.dat stock/Thrinacia.db

    montserrat   ./island config/scheria.dat stock/Scheria.db
                 ./island config/ismarus.dat stock/Ismarus.db
                 ./island config/ogygia.dat stock/Ogygia.db

    qualsevol    ./odysseus config/polyphemus.dat
                 ./odysseus config/telemachus.dat


COMANDES ODYSSEUS

    CONNECT ITHACA
    LIST VOYAGES
    ACCEPT <id>
    SAIL <illa>
    LIST MARKET
    BUY <producte> <quantitat>
    SELL <producte> <quantitat>
    BUY MAP 1
    MAP
    STATUS


OBSERVACIONS

- MAP necessita un terminal d'almenys 80 columnes.
- Si es tanca un servidor amb clients connectats, el port queda ocupat uns
  60 segons (TIME_WAIT). Tancar primer els Odysseus i despres els servidors.
