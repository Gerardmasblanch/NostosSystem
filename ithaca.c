/***********************************************
*
* @Proposit: Proces Ithaca: servidor central que gestiona els encarrecs
*            disponibles i custodia els objectes a transportar.
* @Autor/s: Arnau Ricart i Gerard Mas
* @Data creacio: 24/09/2026
* @Data ultima modificacio: 24/09/2026
*
************************************************/

//Llibreries del sistema
#include <signal.h>

//Llibreries propies
#include "utils.h"
#include "types.h"
#include "config.h"

//Procediments i funcions

/***********************************************
*
* @Finalitat: Atendre la senyal SIGINT sense finalitzar el proces de cop, de
*             manera que pause() retorni i el main pugui alliberar recursos.
* @Parametres:  in: senyal = identificador de la senyal rebuda.
* @Retorn: ----.
*
************************************************/

//Variables globals
int sortirprogram = 0;

void handleSignal (int senyal) {
    if (senyal == SIGINT) {
        sortirprogram = 1;
    }
}

//Procediment principal
int main (int argc, char *argv[]) {
    IthacaConfig ithaca_config;
    Voyage *viatges = NULL;
    char *missatge = NULL;
    int n_viatges = 0;

    if (argc != 3) {
        printF(1, "Us: ./ithaca <config.dat> <voyages.dat>\n");
        return -1;
    }

    signal(SIGINT, handleSignal);

    if (readIthacaConfig(argv[1], &ithaca_config) < 0) {
        printF(1, "Error: no s'ha pogut obrir el fitxer de configuracio.\n");
        return -1;
    }

    if (readVoyages(argv[2], &viatges, &n_viatges) < 0) {
        printF(1, "Error: no s'ha pogut obrir el fitxer de viatges.\n");
        freeIthacaConfig(&ithaca_config);
        return -1;
    }

    asprintf(&missatge, "Ithaca initialized. %d voyages loaded.\nWaiting for Odysseus...\n", n_viatges);
    printF(1, missatge);
    free(missatge);

    //Espera passiva: el proces dorm sense consumir CPU fins que arriba SIGINT.
    pause();

    if(sortirprogram) {
        printF(1, "\nIthaca closes the harbor.");

        freeVoyages(viatges, n_viatges);
        freeIthacaConfig(&ithaca_config);

        //Tornem el comportament per defecte de SIGINT i ens la reenviem, de manera
        //que el proces acabi amb el codi de sortida correcte (128 + SIGINT).
        signal(SIGINT, SIG_DFL);
        raise(SIGINT);
    }

    return 0;
}
