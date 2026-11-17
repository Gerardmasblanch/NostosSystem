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
#include "harbor.h"

//Variables globals
int sortirprogram = 0;
int canonada_avis[2];

//Procediments i funcions

/***********************************************
*
* @Finalitat: Activar la bandera de sortida en rebre SIGINT i escriure un
*             byte a la pipe d'avis per despertar tots els select.
* @Parametres:  in: senyal = identificador de la senyal rebuda.
* @Retorn: ----.
*
************************************************/
void handleSignal (int senyal) {
    if (senyal == SIGINT) {
        sortirprogram = 1;
        write(canonada_avis[1], "x", 1);
    }
}

//Procediment principal
int main (int argc, char *argv[]) {

    IslandConfig config;
    Mercat mercat;
    char *missatge = NULL;
    Harbor harbor;
    int fd_servidor = ERR_CODI;


    if (argc != 3) {
        UTILS_printF(1, "Us: ./island <config.dat> <stock.db>\n");
        return -1;
    }

    if (CONFIG_readIslandConfig(argv[1], &config) < 0) {
        UTILS_printF(1, "Error: no s'ha pogut obrir el fitxer de configuracio.\n");
        return -1;
    }

    if (UTILS_esPortValid(config.port) == PORT_NO_VALID) {
        UTILS_printF(1, MSG_PORT_NO_VALID);
        CONFIG_freeIslandConfig(&config);
        return -1;
    }

    // Filtra amb SPHRAGIS les rutes i elimina les que no coneix
    if (CONFIG_filterIslandRoutes(&config) < 0) {
        UTILS_printF(1, "Error: no s'han pogut validar les rutes de l'illa.\n");
        CONFIG_freeIslandConfig(&config);
        return -1;
    }

    if (CONFIG_readStock(argv[2], &mercat.productes, &mercat.n_productes) < 0) {
        UTILS_printF(1, "Error: no s'ha pogut obrir el fitxer de productes.\n");
        CONFIG_freeIslandConfig(&config);
        return -1;
    }

    mercat.fitxer = argv[2];
    fd_servidor = TRAMA_obrirServidor(config.ip, config.port);

    if (fd_servidor == ERR_CODI) {
        UTILS_printF(1, "Error: no s'ha pogut obrir el servidor de l'illa.\n");
        CONFIG_freeStock(mercat.productes);
        CONFIG_freeIslandConfig(&config);
        return -1;
    }

    HARBOR_mostrarInici(&config, mercat.n_productes);

    if (pipe(canonada_avis) < 0) {
        UTILS_printF(1, MSG_ERROR_PIPE);
        close(fd_servidor);
        CONFIG_freeStock(mercat.productes);
        CONFIG_freeIslandConfig(&config);
        return -1;
    }

    signal(SIGINT, handleSignal);
    signal(SIGPIPE, SIG_IGN);

    MARKET_inicialitzarMercat(&mercat);
    HARBOR_inicialitzarHarbor(&harbor, &config, &mercat, canonada_avis[0]);

    HARBOR_executarServidor(fd_servidor, &harbor);

    asprintf(&missatge, "\n%s closes its port.\n", config.name);
    UTILS_printF(1, missatge);
    free(missatge);

    HARBOR_alliberarHarbor(&harbor);
    MARKET_destruirMercat(&mercat);
    close(canonada_avis[0]);
    close(canonada_avis[1]);
    close(fd_servidor);
    CONFIG_freeStock(mercat.productes);
    CONFIG_freeIslandConfig(&config);

    return 0;

}
