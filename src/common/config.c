/***********************************************
*
* @Proposit: Implementacio de la lectura dels fitxers de configuracio.
* @Autor/s: Arnau Ricart i Gerard Mas
* @Data creacio: 24/09/2026
* @Data ultima modificacio: 24/09/2026
*
************************************************/

//Include del .h
#include "config.h"

//Procediments i funcions
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
    config->port = atoi(aux);
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
    Voyage *nous_viatges = NULL;
    int fd = 0;
    char *aux = NULL;

    *viatges = NULL;
    *n_viatges = 0;

    fd = open(nom_fitxer, O_RDONLY);

    if (fd < 0) {
        return -1;
    }

    while((aux = readUntil(fd, ' ')) != NULL) {

        nous_viatges = realloc((*viatges), sizeof(Voyage) * ((*n_viatges) + 1));

        if (nous_viatges == NULL) {
            printF(2, MSG_SENSE_MEMORIA);
            free(aux);
            freeVoyages(*viatges, *n_viatges);
            *viatges = NULL;
            *n_viatges = 0;
            close(fd);
            return -1;
        }

        (*viatges) = nous_viatges;
        (*viatges)[*n_viatges].object = aux;
        (*viatges)[*n_viatges].id = (*n_viatges) + 1;
        (*viatges)[*n_viatges].file = readUntil(fd, ' ');
        (*viatges)[*n_viatges].destination = readUntil(fd, ' ');
        
        aux = readUntil(fd, '\n');
        (*viatges)[*n_viatges].reward = atoi(aux);
        free(aux);
        (*n_viatges)++;
        
    }

    close(fd);
    return 0;
}

void freeVoyages(Voyage *viatges, int n_viatges) {
    int i = 0;
    
    for (i = 0; i < n_viatges; i++) {
        free(viatges[i].object);
        free(viatges[i].file);
        free(viatges[i].destination);
    }
    free(viatges);
}

int readIslandConfig (char *nom_fitxer, IslandConfig *config) {
    Route *noves_rutes = NULL;
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
    config->port = atoi(aux);
    free(aux);

    aux = readUntil(fd, '\n');
    config->capacity = atoi(aux);
    free(aux);
    
    aux = readUntil(fd, '\n');
    free(aux);

    config->routes = NULL;
    config->n_routes = 0;
    

    while((aux = readUntil(fd, ' ')) != NULL) {
        noves_rutes = realloc(config->routes, sizeof(Route) * (config->n_routes + 1));

        if (noves_rutes == NULL) {
            printF(2, MSG_SENSE_MEMORIA);
            free(aux);
            close(fd);
            return -1;
        }

        config->routes = noves_rutes;
        config->routes[config->n_routes].name = aux;
        config->routes[config->n_routes].ip = readUntil(fd, ' ');

        aux = readUntil(fd, '\n');
        config->routes[config->n_routes].port = atoi(aux);
        free(aux);

        config->n_routes++;
    }

    close(fd);
    return 0;

}

void freeIslandConfig (IslandConfig *config) {
    int i = 0;
    
    free(config->name);
    free(config->folder);
    free(config->ip);

    for (i = 0; i < config->n_routes; i++) {
        free(config->routes[i].name);
        free(config->routes[i].ip);
    }
    free(config->routes);

}

int readStock(char *nom_fitxer, Product **productes, int *n_productes) {
    Product *nous_productes = NULL;
    Product producte;
    int fd = 0;

    *productes = NULL;
    *n_productes = 0;

    fd = open(nom_fitxer, O_RDONLY);

    if (fd < 0) {
        return -1;
    }

    while(read(fd,&producte,sizeof(Product)) == sizeof(Product)) {
        nous_productes = realloc((*productes), sizeof(Product) * ((*n_productes) + 1));

        if (nous_productes == NULL) {
            printF(2, MSG_SENSE_MEMORIA);
            freeStock(*productes);
            *productes = NULL;
            *n_productes = 0;
            close(fd);
            return -1;
        }

        (*productes) = nous_productes;
        (*productes)[*n_productes] = producte;
        (*n_productes)++;
    }

    close(fd);
    return 0;
}

void freeStock(Product *productes) {
    free(productes);
}

int filterIslandRoutes (IslandConfig *config) {
    SPHRAGIS_Island illa;
    Route *rutes_valides;
    int n_valides;
    int sobreviu;
    int i;
    int j;
    int k;

    // Us del SPHRAGIS 
    illa.name = config->name;
    illa.known_island_count = config->n_routes;
    illa.known_islands = malloc(sizeof(char *) * config->n_routes);

    if (illa.known_islands == NULL) {
        printF(2, MSG_SENSE_MEMORIA);
        return SPHRAGIS_ERROR;
    }

    for (i = 0; i < config->n_routes; i++) {
        illa.known_islands[i] = config->routes[i].name;
    }

    n_valides = SPHRAGIS_filter_island_configuration(&illa);

    if (n_valides < 0) {
        free(illa.known_islands);
        return n_valides;
    }

    rutes_valides = malloc(sizeof(Route) * n_valides);

    if (rutes_valides == NULL) {
        printF(2, MSG_SENSE_MEMORIA);
        free(illa.known_islands);
        return SPHRAGIS_ERROR;
    }

    j = 0;

    for (i = 0; i < config->n_routes; i++) {
        sobreviu = 0;

        for (k = 0; k < n_valides; k++) {
            if (illa.known_islands[k] == config->routes[i].name) {
                sobreviu = 1;
            }
        }

        if (sobreviu == 1) {
            rutes_valides[j] = config->routes[i];
            j++;
        } else {
            free(config->routes[i].ip);
        }
    }

    free(config->routes);
    free(illa.known_islands);

    config->routes = rutes_valides;
    config->n_routes = n_valides;

    return n_valides;
}

int readOdysseusConfig (char *nom_fitxer, OdysseusConfig *config) {
    
    int fd = 0;
    char *aux = NULL;
    int i = 0;

    fd = open(nom_fitxer, O_RDONLY);

    if (fd < 0) {
        return -1;
    }

    config->folder = readUntil(fd, '\n');
    config->name = readUntil(fd, ' ');
    config->ithaca_ip = readUntil(fd, ' ');
    aux = readUntil(fd, '\n');
    config->ithaca_port = atoi(aux);
    free(aux);

    config->aeaea_ip = readUntil(fd, ' ');
    aux = readUntil(fd, '\n');
    config->aeaea_port = atoi(aux);
    free(aux);

    aux = readUntil(fd, '\n');
    config->gold = atoi(aux);
    free(aux);

    aux = readUntil(fd, '\n');
    config->n_foods = atoi(aux);
    free(aux);
    config->foods = malloc(sizeof(Food) * config->n_foods);

    if (config->foods == NULL) {
        printF(2, MSG_SENSE_MEMORIA);
        close(fd);
        return -1;
    }

    for (i = 0; i < config->n_foods; i++) {

        config->foods[i].name = readUntil(fd, ' ');
        aux = readUntil(fd, '\n');
        config->foods[i].amount = atoi(aux);
        free(aux);
    }

    close(fd);

    return 0;
}

void freeOdysseusConfig (OdysseusConfig *config) {
    int i = 0;

    free(config->folder);
    free(config->name);
    free(config->ithaca_ip);
    free(config->aeaea_ip);

    for (i = 0; i < config->n_foods; i++) {
        free(config->foods[i].name);
    }

    free(config->foods);
    
}