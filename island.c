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
    Product *productes = NULL;
    char *missatge = NULL;
    int n_productes = 0;

    if (argc != 3) {
        printF(1, "Us: ./island <config.dat> <stock.db>\n");
        return -1;
    }

    signal(SIGINT, handleSignal);

    if (readIslandConfig(argv[1], &config) < 0) {
        printF(1, "Error: no s'ha pogut obrir el fitxer de configuracio.\n");
        return -1;
    }

    // Filtra amb SPHRAGIS les rutes i elimina les que no coneix
    if (filterIslandRoutes(&config) < 0) {
        printF(1, "Error: no s'han pogut validar les rutes de l'illa.\n");
        freeIslandConfig(&config);
        return -1;
    }

    if (readStock(argv[2], &productes, &n_productes) < 0) {
        printF(1, "Error: no s'ha pogut obrir el fitxer de productes.\n");
        freeIslandConfig(&config);
        return -1;
    }

    asprintf(&missatge,"Island %s initialized.\nPort capacity: %d ship.\n%d sea routes loaded.\n%d products available.\n",config.name, config.capacity, config.n_routes, n_productes);
    printF(1, missatge);
    free(missatge);


    pause();

    // SIGINT ctrl+c
    if(sortirprogram) {

        asprintf(&missatge, "\n%s closes its port.\n", config.name);
        printF(1, missatge);
        free(missatge);

        freeStock(productes);
        freeIslandConfig(&config);

        signal(SIGINT, SIG_DFL);
        raise(SIGINT);
    }

    return 0;
}
