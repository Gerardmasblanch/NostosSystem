/***********************************************
*
* @Proposit: Proces Ithaca, servidor de viatges, que gestiona els encarrecs dels Odysseus i els envia a les illes.
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
#include "trama.h"

//Define 
#define MSG_ERROR_SERVIDOR "Error: no s'ha pogut obrir el servidor d'Ithaca.\n"  // això haura d'anar algun .h crec 

//Variables globals
int sortirprogram = 0;

//Procediments i funcions

/***********************************************
*
* @Finalitat: Atendre la senyal SIGINT sense finalitzar el proces de cop, de
*             manera que pause() retorni i el main pugui alliberar recursos.
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
    IthacaConfig ithaca_config;
    Voyage *viatges = NULL;
    char *missatge = NULL;
    int n_viatges = 0;
    int fd_server = ERR_CODI;  // això fica un -1 per defecte que es error

    if (argc != 3) {
        printF(1, "Us: ./ithaca <config.dat> <voyages.dat>\n");
        return -1;
    }

    signal(SIGINT, handleSignal);

    if (readIthacaConfig(argv[1], &ithaca_config) < 0) {
        printF(1, "Error: no s'ha pogut obrir el fitxer de configuracio.\n");
        return -1;
    }

    if (esPortValid(ithaca_config.port) == PORT_NO_VALID) {
        printF(1, MSG_PORT_NO_VALID);
        freeIthacaConfig(&ithaca_config);
        return -1;
    }
    
    if (readVoyages(argv[2], &viatges, &n_viatges) < 0) {
        printF(1, "Error: no s'ha pogut obrir el fitxer de viatges.\n");
        freeIthacaConfig(&ithaca_config);
        return -1;
    }

    fd_server = obrirServidor(ithaca_config.ip, ithaca_config.port);

    if (fd_server == ERR_CODI) {
        printF(1, MSG_ERROR_SERVIDOR);
        freeVoyages(viatges, n_viatges);
        freeIthacaConfig(&ithaca_config);
        return -1;
    }

    asprintf(&missatge, "Ithaca initialized. %d voyages loaded.\nWaiting for Odysseus...\n", n_viatges);
    printF(1, missatge);
    free(missatge);

    pause();

    //SIGNT ctrl+c
    if(sortirprogram) {
        printF(1, "\nIthaca closes the harbor.\n");

        close(fd_server);
        freeVoyages(viatges, n_viatges);
        freeIthacaConfig(&ithaca_config);

        signal(SIGINT, SIG_DFL);
        raise(SIGINT);
    }

    return 0;
}
