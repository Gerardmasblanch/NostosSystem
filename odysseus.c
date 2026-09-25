/***********************************************
*
* @Proposit: Proces Odysseus: llegeix la seva configuracio i ofereix el
*            terminal interactiu de comandes del navegant.
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
#include "commands.h"

//Procediments i funcions

/***********************************************
*
* @Finalitat: Atendre la senyal SIGINT sense finalitzar el proces de cop. En
*             rebre-la, el read() que espera el teclat s'interromp i retorna
*             error, cosa que fa sortir del bucle del terminal.
* @Parametres:  in: senyal = identificador de la senyal rebuda.
* @Retorn: ----.
*
************************************************/
void handleSignal (int senyal) {
    (void)senyal;
}

//Procediment principal
int main (int argc, char *argv[]) {
    OdysseusConfig config;
    char *missatge = NULL;
    char *linia = NULL;

    if (argc != 2) {
        printF(1, "Us: ./odysseus <config.dat>\n");
        return -1;
    }

    signal(SIGINT, handleSignal);

    if (readOdysseusConfig(argv[1], &config) < 0) {
        printF(1, "Error: no s'ha pogut obrir el fitxer de configuracio.\n");
        return -1;
    }

    asprintf(&missatge, "Odysseus %s is ready to sail.\n\n", config.name);
    printF(1, missatge);
    free(missatge);

    while (1) {
        printF(1, "$ ");

        linia = readUntil(0, '\n');

        //Amb SIGINT el read del teclat s'interromp i readUntil retorna NULL,
        //de manera que sortim del bucle i podem alliberar els recursos.
        if (linia == NULL) {
            break;
        }

        //TODO: cridar parseCommand(linia) i respondre segons el codi retornat.

        free(linia);
    }

    //TODO: descomentar quan freeOdysseusConfig estigui implementada.
    //freeOdysseusConfig(&config);

    //Tornem el comportament per defecte de SIGINT i ens la reenviem, de manera
    //que el proces acabi amb el codi de sortida correcte (128 + SIGINT).
    signal(SIGINT, SIG_DFL);
    raise(SIGINT);

    return 0;
}
