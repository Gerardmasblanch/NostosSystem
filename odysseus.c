/***********************************************
*
* @Proposit: Proces Odysseus: llegeix la seva configuracio i ofereix el
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

//Procediment principal
int main (int argc, char *argv[]) {
    Navegant navegant;
    char *missatge = NULL;

    if (argc != 2) {
        printF(1, "Us: ./odysseus <config.dat>\n");
        return -1;
    }

    if (readOdysseusConfig(argv[1], &navegant.config) < 0) {
        printF(1, "Error: could not open configuration file.\n");
        return -1;
    }

    if (esPortValid(navegant.config.ithaca_port) == PORT_NO_VALID || esPortValid(navegant.config.aeaea_port) == PORT_NO_VALID) {
        printF(1, MSG_PORT_NO_VALID);
        freeOdysseusConfig(&navegant.config);
        return -1;
    }

    if (inicialitzarNavegant(&navegant) == ERR_CODI) {
        alliberarNavegant(&navegant);
        return -1;
    }

    signal(SIGINT, handleSignal);
    signal(SIGPIPE, SIG_IGN);

    asprintf(&missatge, "Odysseus %s is ready to sail.\n\n", navegant.config.name);
    printF(1, missatge);
    free(missatge);

    executarOdysseus(&navegant, &sortirprogram);

    alliberarNavegant(&navegant);

    if (sortirprogram == 1) {
        printF(1, "\n");
        signal(SIGINT, SIG_DFL);
        raise(SIGINT);
    }

    return 0;
}