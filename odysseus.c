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

//Variables globals
//El handler de SIGINT no pot rebre parametres, i el terminal es queda
//bloquejat dins d'un read() que signal() reinicia automaticament, de manera
//que la unica forma d'acabar es alliberar i morir des del propi handler.
//L'Annex III de l'enunciat permet variables globals als processos principals.
OdysseusConfig config;

//Procediments i funcions

/***********************************************
*
* @Finalitat: Atendre la senyal SIGINT alliberant els recursos del proces i
*             finalitzant-lo amb el codi de sortida corresponent a la senyal.
* @Parametres:  in: senyal = identificador de la senyal rebuda.
* @Retorn: ----.
*
************************************************/
void handleSignal (int senyal) {
    printF(1, "\n");

    //TODO: descomentar quan freeOdysseusConfig estigui implementada.
    //freeOdysseusConfig(&config);

    //Tornem el comportament per defecte de la senyal i ens la reenviem, de
    //manera que el proces acabi amb el codi de sortida correcte (128 + senyal).
    signal(senyal, SIG_DFL);
    raise(senyal);
}

//Procediment principal
int main (int argc, char *argv[]) {
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

        //Final d'entrada (CTRL+D): sortim del bucle de forma controlada.
        if (linia == NULL) {
            break;
        }

        //TODO: cridar parseCommand(linia) i respondre segons el codi retornat.

        free(linia);
    }

    //TODO: descomentar quan freeOdysseusConfig estigui implementada.
    //freeOdysseusConfig(&config);

    return 0;
}
