/***********************************************
*
* @Proposit: Implementacio de la lectura dels fitxers de configuracio.
* @Autor/s: Arnau Ricart i Gerard Mas
* @Data creacio: 24/09/2026
* @Data ultima modificacio: 24/09/2026
*
************************************************/

#include "config.h"


int readIthacaConfig (char *nom_fitxer, IthacaConfig *config) {
    
    int fd = 0;
    char *aux = NULL;

    fd = open(nom_fitxer, O_RDONLY);

    if (fd < 0) {
        return -1;
    }

    config->name = readUntil(fd, '\n');
    config->folder = readUntil(fd, '\n');
    config->ip = readUntil(fd, ' ');

    aux = readUntil(fd, '\n');
    config->port = parseInt(aux);
    free(aux);
    close(fd);

    return 0;
}

void freeIthacaConfig (IthacaConfig *config) {
    free(config->name);
    free(config->folder);
    free(config->ip);
}

int readVoyages(char *nom_fitxer, Voyage **viatges, int *n_viatges) {
    

    int fd = 0;
    char *aux = NULL;
    *viatges = NULL;
    *n_viatges = 0;

    fd = open(nom_fitxer, O_RDONLY);

    if (fd < 0) {
        return -1;
    }

    while((aux = readUntil(fd, ' ')) != NULL) {

        (*viatges) = realloc((*viatges), sizeof(Voyage) * ((*n_viatges) + 1));
        (*viatges)[*n_viatges].object = aux;
        (*viatges)[*n_viatges].id = (*n_viatges) + 1;
        (*viatges)[*n_viatges].file = readUntil(fd, ' ');
        (*viatges)[*n_viatges].destination = readUntil(fd, ' ');
        
        aux = readUntil(fd, '\n');
        (*viatges)[*n_viatges].reward = parseInt(aux);
        free(aux);
        (*n_viatges)++;
        
    }

    close(fd);
    return 0;
}

void freeVoyages(Voyage *viatges, int n_viatges) {
    for (int i = 0; i < n_viatges; i++) {
        free(viatges[i].object);
        free(viatges[i].file);
        free(viatges[i].destination);
    }
    free(viatges);
}

