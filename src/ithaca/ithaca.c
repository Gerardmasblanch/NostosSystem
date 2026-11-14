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
#include "voyages.h"


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
    
    ServidorIthaca servidor;
    int resultat;
    
    IthacaConfig ithaca_config;
    Voyage *viatges = NULL;
    char *missatge = NULL;
    int n_viatges = 0;
    int fd_server = ERR_CODI;  

    if (argc != 3) {
        UTILS_printF(1, "Us: ./ithaca <config.dat> <voyages.dat>\n");
        return -1;
    }

    signal(SIGINT, handleSignal);
    signal(SIGPIPE, SIG_IGN); 

    if (CONFIG_readIthacaConfig(argv[1], &ithaca_config) < 0) {
        UTILS_printF(1, "Error: no s'ha pogut obrir el fitxer de configuracio.\n");
        return -1;
    }

    if (UTILS_esPortValid(ithaca_config.port) == PORT_NO_VALID) {
        UTILS_printF(1, MSG_PORT_NO_VALID);
        CONFIG_freeIthacaConfig(&ithaca_config);
        return -1;
    }
    
    if (CONFIG_readVoyages(argv[2], &viatges, &n_viatges) < 0) {
        UTILS_printF(1, "Error: no s'ha pogut obrir el fitxer de viatges.\n");
        CONFIG_freeIthacaConfig(&ithaca_config);
        return -1;
    }

    fd_server = TRAMA_obrirServidor(ithaca_config.ip, ithaca_config.port);

    if (fd_server == ERR_CODI) {
        UTILS_printF(1, MSG_ERROR_SERVIDOR);
        CONFIG_freeVoyages(viatges, n_viatges);
        CONFIG_freeIthacaConfig(&ithaca_config);
        return -1;
    }

    asprintf(&missatge, "Ithaca initialized. %d voyages loaded.\nWaiting for Odysseus...\n", n_viatges);
    UTILS_printF(1, missatge);
    free(missatge);

    VOYAGES_inicialitzarServidorIthaca(&servidor);

    if (VOYAGES_prepararViatgesIthaca(&servidor, viatges, n_viatges) == ERR_CODI) {
        VOYAGES_alliberarServidorIthaca(&servidor);
        close(fd_server);
        CONFIG_freeVoyages(viatges, n_viatges);
        CONFIG_freeIthacaConfig(&ithaca_config);
        return ERR_CODI;
    }

    resultat = VOYAGES_executarServidorIthaca(fd_server, &servidor, &sortirprogram);

    if (sortirprogram == 1) {
        UTILS_printF(1, "\nIthaca closes the harbor.\n");
    }

    VOYAGES_alliberarServidorIthaca(&servidor);
    close(fd_server);
    CONFIG_freeVoyages(viatges, n_viatges);
    CONFIG_freeIthacaConfig(&ithaca_config);

    if (sortirprogram == 1) {
        signal(SIGINT, SIG_DFL);
        raise(SIGINT);
    }

    return resultat;
}

