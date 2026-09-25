/***********************************************
*
* @Proposit: Proces Island: representa una illa de l'arxipelag amb el seu
*            port, el seu mercat i les seves connexions maritimes.
* @Autor/s: Arnau Ricart i Gerard Mas
* @Data creacio: 24/09/2026
* @Data ultima modificacio: 24/09/2026
*
************************************************/
#include <signal.h>

//Llibreries propies
#include "utils.h"
#include "types.h"
#include "config.h"

void handleSignal (int senyal) {
    (void)senyal;
}

//Procediment principal
int main (int argc, char *argv[]) {
    
    IslandConfig config;
    Product *productes = NULL;
    char *missatge = NULL;
    int n_productes = 0;

    if (argc != 3) {
        printF(1, "Use: ./island <config.dat> <stock.db>\n");
        return -1;
    }

    signal(SIGINT, handleSignal);

    if(readIslandConfig(argv[1], &config) < 0) {
        printF(1, "Error: no s'ha pogut obrir el fitxer de configuracio.\n");
        return -1;
    }


    if(readStock(argv[2], &productes, &n_productes) < 0) {
        printF(1, "Error: no s'ha pogut obrir el fitxer de productes.\n");
        freeIslandConfig(&config);
        return -1;
    }

    pause(); //Espera passiva: el proces dorm sense consumir CPU fins que arriba SIGINT.

    asprintf(&missatge,"Island %s initialized.\nPort capacity: %d ship.\n%d sea routes loaded.\n%d products available.\n",config.name, config.capacity, config.n_routes, n_productes);
    printF(1, missatge);
    free(missatge);
    freeIslandConfig(&config);

    signal(SIGINT, SIG_IGN); //Ignora la senyal SIGINT per evitar que el procés es tanqui abans d'alliberar recursos.
    raise(SIGINT); //Envia la senyal SIGINT a si mateix per que el procés es tanqui.

    return 0;
}
