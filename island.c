/***********************************************
*
* @Proposit: Proces Island representa una illa amb el seu port i el seu comerç.
* @Autor/s: Arnau Ricart i Gerard Mas
* @Data creacio: 24/09/2026
* @Data ultima modificacio: 25/09/2026
*
************************************************/

//Llibreries del sistema
#include <signal.h>

//Llibreries propies
#include "utils.h"
#include "types.h"
#include "config.h"
#include "harbor.h"

//Variables globals
int sortirprogram = 0;

//Procediments i funcions

/***********************************************
*
* @Finalitat: Atendre la senyal SIGINT sense finalitzar el proces de cop i lliurar tota la memoria.
* @Parametres:  in: senyal = identificador de la senyal rebuda.
* @Retorn: ----.
*
************************************************/

void handleSignal (int senyal) {
    if (senyal == SIGINT) {
        sortirprogram = 1;
    }
}

//Procediment principal
int main (int argc, char *argv[]) {

    IslandConfig config;
    Mercat mercat;
    char *missatge = NULL;
    Harbor harbor;
    int fd_servidor = ERR_CODI;


    if (argc != 3) {
        printF(1, "Us: ./island <config.dat> <stock.db>\n");
        return -1;
    }

    signal(SIGINT, handleSignal);
    signal(SIGPIPE, SIG_IGN);
    
    if (readIslandConfig(argv[1], &config) < 0) {
        printF(1, "Error: no s'ha pogut obrir el fitxer de configuracio.\n");
        return -1;
    }

    if (esPortValid(config.port) == PORT_NO_VALID) {
        printF(1, MSG_PORT_NO_VALID);
        freeIslandConfig(&config);
        return -1;
    }

    // Filtra amb SPHRAGIS les rutes i elimina les que no coneix
    if (filterIslandRoutes(&config) < 0) {
        printF(1, "Error: no s'han pogut validar les rutes de l'illa.\n");
        freeIslandConfig(&config);
        return -1;
    }

    if (readStock(argv[2], &mercat.productes, &mercat.n_productes) < 0) {
        printF(1, "Error: no s'ha pogut obrir el fitxer de productes.\n");
        freeIslandConfig(&config);
        return -1;
    }

    mercat.fitxer = argv[2];
    fd_servidor = obrirServidor(config.ip, config.port);

    if (fd_servidor == ERR_CODI) {
        printF(1, "Error: no s'ha pogut obrir el servidor de l'illa.\n");
        freeStock(mercat.productes);
        freeIslandConfig(&config);
        return -1;
    }

    mostrarInici(&config, mercat.n_productes);

    inicialitzarHarbor(&harbor);
    executarServidor(fd_servidor, &harbor, &config, &mercat, &sortirprogram);

    asprintf(&missatge, "\n%s closes its port.\n", config.name);
    printF(1, missatge);
    free(missatge);

    alliberarHarbor(&harbor);
    close(fd_servidor);
    freeStock(mercat.productes);
    freeIslandConfig(&config);

    signal(SIGINT, SIG_DFL);
    raise(SIGINT);

    return 0;

}
