/***********************************************
*
* @Proposit: Proces Odysseus, llegeix la seva configuracio i ofereix el
*            terminal interactiu de comandes del navegant.
* @Autor/s: Arnau Ricart i Gerard Mas
* @Data creacio: 24/09/2026
* @Data ultima modificacio: 29/10/2026
*
************************************************/

//Llibreries del sistema
#include <signal.h>

//Llibreries propies
#include "utils.h"
#include "config.h"
#include "navegant.h"
#include "travessia.h"

//Variables globals
int sortirprogram = 0;
int racio_pendent = 0;

//Procediments i funcions

/***********************************************
*
* @Finalitat: Activar la bandera de sortida en rebre SIGINT.
* @Parametres:  in: senyal = identificador de la senyal rebuda.
* @Retorn: ----.
*
************************************************/
void handleSignal (int senyal) {
    if (senyal == SIGINT) {
        sortirprogram = 1;
    }
}

/***********************************************
*
* @Finalitat: Activar la bandera de racio pendent en rebre SIGALRM.
* @Parametres:  in: senyal = identificador de la senyal rebuda.
* @Retorn: ----.
*
************************************************/
void handleAlarma (int senyal) {
    if (senyal == SIGALRM) {
        racio_pendent = 1;
    }
}

//Procediment principal
int main (int argc, char *argv[]) {
    Navegant navegant;
    char *missatge = NULL;

    if (argc != 2) {
        UTILS_printF(1, "Us: ./odysseus <config.dat>\n");
        return -1;
    }

    if (CONFIG_readOdysseusConfig(argv[1], &navegant.config) < 0) {
        UTILS_printF(1, "Error: could not open configuration file.\n");
        return -1;
    }

    if (UTILS_esPortValid(navegant.config.ithaca_port) == PORT_NO_VALID || UTILS_esPortValid(navegant.config.aeaea_port) == PORT_NO_VALID) {
        UTILS_printF(1, MSG_PORT_NO_VALID);
        CONFIG_freeOdysseusConfig(&navegant.config);
        return -1;
    }

    if (NAVEGANT_inicialitzarNavegant(&navegant) == ERR_CODI) {
        NAVEGANT_alliberarNavegant(&navegant);
        return -1;
    }

    signal(SIGINT, handleSignal);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGALRM, handleAlarma);

    asprintf(&missatge, "Odysseus %s is ready to sail.\n\n", navegant.config.name);
    UTILS_printF(1, missatge);
    free(missatge);

    TRAVESSIA_executarOdysseus(&navegant, &sortirprogram, &racio_pendent);

    NAVEGANT_alliberarNavegant(&navegant);

    if (sortirprogram == 1) {
        UTILS_printF(1, "\n");
        signal(SIGINT, SIG_DFL);
        raise(SIGINT);
    }

    return 0;
}