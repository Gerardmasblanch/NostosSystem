/***********************************************
*
* @Proposit: Proces Odysseus: llegeix la seva configuracio i ofereix el
*            terminal interactiu de comandes del navegant.
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
#include "commands.h"
#include "unistd.h"

//Variables globals
int sortirPrograma = 0;

void handleSignal(int senyal) {
    if (senyal == SIGINT) {
        sortirPrograma = 1;
        close(0);  // Aixo no es pot fer arreglar
    }
}


//Procediment principal
int main (int argc, char *argv[]) {
    
    OdysseusConfig config;
    char *missatge = NULL;
    char *linea = NULL;
    int codi = 0;

    if(argc != 2) {
        printF(1, "Use: ./odysseus <config.dat>\n");
        return -1;
    }

    if(readOdysseusConfig(argv[1], &config) < 0) {
        printF(1, "Error: could not open configuration file.\n");
        return -1;
    }

     signal(SIGINT, handleSignal);

    asprintf(&missatge, "Odysseus %s is ready to sail.\n\n", config.name);
    printF(1, missatge);
    free(missatge);

    while(1){

        printF(1, "$ ");
        linea = readUntil(0, '\n');

        //Final d'entrada (CTRL+D): sense aquesta comprovacio, readUntil retorna     <-- AIXO HO HE TRET PA QUE 
        //NULL indefinidament i el terminal entra en un bucle infinit.
        
        if (sortirPrograma) {   // <-- SIGINT pero amb el exit(0) no pasa pera aqui
            break;
        }

        //parsejar i respondre
        codi = parseCommand(linea);
        if(codi == CMD_DESCONEGUDA) {
            printF(1, "Unknown command\n\n");
        } else if(codi != CMD_ERROR_SINTAXI) {
            printF(1, "Command OK\n\n");
        }
        
        free(linea);    
    }

    freeOdysseusConfig(&config);

    return 0;
}
