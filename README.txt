THE NOSTOS SYSTEM - FASE 3
Grup 1: Arnau Ricart (arnau.ricart) i Gerard Mas (gerard.mb)


ESTRUCTURA

    src/common     moduls compartits pels tres processos
                   (utils, config, trama, fitxer, types, sphragis)
    src/odysseus   proces Odysseus (odysseus, commands, navegant, travessia)
    src/ithaca     proces Ithaca (ithaca, voyages)
    src/island     proces Island (island, harbor, market)
    config         fitxers de configuracio
    stock          fitxers stock.db de les illes
    objects        imatges dels objectes dels viatges (carpeta d'Ithaca)
    *_files        carpetes de cada illa i de cada Odysseus

    Les funcions publiques de cada modul porten el prefix del modul:
    UTILS_, CONFIG_, TRAMA_, FITXER_, COMMANDS_, NAVEGANT_, TRAVESSIA_,
    VOYAGES_, HARBOR_ i MARKET_.


COMPILAR

    make
    make clean

    Els executables es generen a l'arrel del projecte.


EXECUTAR (configuracio de la Testing Global)

    montserrat   ./ithaca config/ithaca.dat config/voyages.dat
                 ./island config/thrinacia.dat stock/Thrinacia.db
                 ./odysseus config/polyphemus.dat
                 ./odysseus config/eumaeus.dat

    matagalls    ./island config/aeaea.dat stock/Aeaea.db
                 ./island config/aeolia.dat stock/Aeolia.db
                 ./odysseus config/telemachus.dat

    puigpedros   ./island config/scheria.dat stock/Scheria.db
                 ./island config/ogygia.dat stock/Ogygia.db
                 ./island config/ismarus.dat stock/Ismarus.db
                 ./odysseus config/penelope.dat

    Amb valgrind:
                 make valgrind-ithaca
                 make valgrind-island      (Aeaea)
                 make valgrind-odysseus    (Polyphemus)


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
    DELIVER
    CLAIM


OBSERVACIONS

- MAP necessita un terminal d'almenys 80 columnes.
- Les carpetes objects i *_files han d'existir (ja venen amb el projecte).
- L'estat de cada Odysseus es desa a <carpeta>/estat.db i es recupera en
  tornar-lo a executar. Si es perd al mar o es queda sense sortida, torna a
  comencar amb els valors del seu fitxer de configuracio.
- Si es tanca un servidor amb clients connectats, el port queda ocupat uns
  60 segons (TIME_WAIT). Tancar primer els Odysseus i despres els servidors.
