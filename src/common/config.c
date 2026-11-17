/***********************************************
*
* @Proposit: Implementacio de la lectura dels fitxers de configuracio.
* @Autor/s: Arnau Ricart i Gerard Mas
* @Data creacio: 24/09/2026
* @Data ultima modificacio: 17/11/2026
*
************************************************/

//Include del .h
#include "config.h"

//Procediments i funcions
int CONFIG_llegirEnter (int fd, char delimitador, int *valor) {
    char *aux = NULL;

    aux = UTILS_readUntil(fd, delimitador);

    if (aux == NULL) {
        return -1;
    }

    *valor = atoi(aux);
    free(aux);

    return 0;
}

int CONFIG_readIthacaConfig (char *nom_fitxer, IthacaConfig *config) {

    int fd = 0;

    fd = open(nom_fitxer, O_RDONLY);

    if (fd < 0) {
        return -1;
    }

    config->name = UTILS_readUntil(fd, '\n');
    config->folder = UTILS_readUntil(fd, '\n');
    config->ip = UTILS_readUntil(fd, ' ');

    if (config->name == NULL || config->folder == NULL || config->ip == NULL || CONFIG_llegirEnter(fd, '\n', &config->port) < 0) {
        CONFIG_freeIthacaConfig(config);
        close(fd);
        return -1;
    }

    close(fd);

    return 0;
}


void CONFIG_freeIthacaConfig (IthacaConfig *config) {
    free(config->name);
    free(config->folder);
    free(config->ip);
}


int CONFIG_readVoyages(char *nom_fitxer, Voyage **viatges, int *n_viatges) {
    Voyage *nous_viatges = NULL;
    int fd = 0;
    char *aux = NULL;
    int estat_lectura = LECTURA_OK;
    int i;

    *viatges = NULL;
    *n_viatges = 0;

    fd = open(nom_fitxer, O_RDONLY);

    if (fd < 0) {
        return -1;
    }

    while (estat_lectura == LECTURA_OK && (aux = UTILS_readUntil(fd, ' ')) != NULL) {

        nous_viatges = realloc((*viatges), sizeof(Voyage) * ((*n_viatges) + 1));

        if (nous_viatges == NULL) {
            UTILS_printF(2, MSG_SENSE_MEMORIA);
            free(aux);
            estat_lectura = LECTURA_ERROR;
        } else {
            i = *n_viatges;
            (*viatges) = nous_viatges;
            (*viatges)[i].object = aux;
            (*viatges)[i].id = i + 1;
            (*viatges)[i].file = UTILS_readUntil(fd, ' ');
            (*viatges)[i].destination = UTILS_readUntil(fd, ' ');
            (*n_viatges)++;

            //Linia en blanc al final del fitxer: no es cap viatge.
            if ((*viatges)[i].file == NULL) {
                free(aux);
                (*n_viatges)--;
                estat_lectura = LECTURA_FINAL;
            } else if ((*viatges)[i].destination == NULL || CONFIG_llegirEnter(fd, '\n', &(*viatges)[i].reward) < 0) {
                estat_lectura = LECTURA_ERROR;
            }
        }
    }

    close(fd);

    if (estat_lectura == LECTURA_ERROR) {
        CONFIG_freeVoyages(*viatges, *n_viatges);
        *viatges = NULL;
        *n_viatges = 0;
        return -1;
    }

    return 0;
}

void CONFIG_freeVoyages(Voyage *viatges, int n_viatges) {
    int i = 0;
    
    for (i = 0; i < n_viatges; i++) {
        free(viatges[i].object);
        free(viatges[i].file);
        free(viatges[i].destination);
    }
    free(viatges);
}

int CONFIG_readIslandConfig (char *nom_fitxer, IslandConfig *config) {
    Route *noves_rutes = NULL;
    int fd = 0;
    char *aux = NULL;
    int estat_lectura = LECTURA_OK;
    int i;

    fd = open(nom_fitxer, O_RDONLY);

    if (fd < 0) {
        return -1;
    }

    config->name = UTILS_readUntil(fd, '\n');
    config->folder = UTILS_readUntil(fd, '\n');
    config->ip = UTILS_readUntil(fd, ' ');
    config->routes = NULL;
    config->n_routes = 0;

    if (config->name == NULL || config->folder == NULL || config->ip == NULL || CONFIG_llegirEnter(fd, '\n', &config->port) < 0 || CONFIG_llegirEnter(fd, '\n', &config->capacity) < 0) {
        CONFIG_freeIslandConfig(config);
        close(fd);
        return -1;
    }

    aux = UTILS_readUntil(fd, '\n');
    free(aux);

    while (estat_lectura == LECTURA_OK && (aux = UTILS_readUntil(fd, ' ')) != NULL) {
        noves_rutes = realloc(config->routes, sizeof(Route) * (config->n_routes + 1));

        if (noves_rutes == NULL) {
            UTILS_printF(2, MSG_SENSE_MEMORIA);
            free(aux);
            estat_lectura = LECTURA_ERROR;
        } else {
            i = config->n_routes;
            config->routes = noves_rutes;
            config->routes[i].name = aux;
            config->routes[i].ip = UTILS_readUntil(fd, ' ');
            config->n_routes++;

            //Linia en blanc al final del fitxer: no es cap ruta.
            if (config->routes[i].ip == NULL) {
                free(aux);
                config->n_routes--;
                estat_lectura = LECTURA_FINAL;
            } else if (CONFIG_llegirEnter(fd, '\n', &config->routes[i].port) < 0) {
                estat_lectura = LECTURA_ERROR;
            }
        }
    }

    close(fd);

    if (estat_lectura == LECTURA_ERROR) {
        CONFIG_freeIslandConfig(config);
        return -1;
    }

    return 0;

}

void CONFIG_freeIslandConfig (IslandConfig *config) {
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

int CONFIG_readStock(char *nom_fitxer, Product **productes, int *n_productes) {
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
            UTILS_printF(2, MSG_SENSE_MEMORIA);
            CONFIG_freeStock(*productes);
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

void CONFIG_freeStock(Product *productes) {
    free(productes);
}

int CONFIG_filterIslandRoutes (IslandConfig *config) {
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
        UTILS_printF(2, MSG_SENSE_MEMORIA);
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
        UTILS_printF(2, MSG_SENSE_MEMORIA);
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

int CONFIG_readOdysseusConfig (char *nom_fitxer, OdysseusConfig *config) {
    
    int fd = 0;
    int estat_lectura = LECTURA_OK;
    int i = 0;

    fd = open(nom_fitxer, O_RDONLY);

    if (fd < 0) {
        return -1;
    }

    config->aeaea_ip = NULL;
    config->foods = NULL;
    config->n_foods = 0;

    config->folder = UTILS_readUntil(fd, '\n');
    config->name = UTILS_readUntil(fd, ' ');
    config->ithaca_ip = UTILS_readUntil(fd, ' ');

    if (config->folder == NULL || config->name == NULL || config->ithaca_ip == NULL || CONFIG_llegirEnter(fd, '\n', &config->ithaca_port) < 0) {
        estat_lectura = LECTURA_ERROR;
    }

    if (estat_lectura == LECTURA_OK) {
        config->aeaea_ip = UTILS_readUntil(fd, ' ');

        if (config->aeaea_ip == NULL || CONFIG_llegirEnter(fd, '\n', &config->aeaea_port) < 0 || CONFIG_llegirEnter(fd, '\n', &config->gold) < 0 || CONFIG_llegirEnter(fd, '\n', &config->n_foods) < 0 || config->n_foods < 0) {
            estat_lectura = LECTURA_ERROR;
        }
    }

    if (estat_lectura == LECTURA_OK) {
        config->foods = malloc(sizeof(Food) * config->n_foods);

        if (config->foods == NULL && config->n_foods > 0) {
            UTILS_printF(2, MSG_SENSE_MEMORIA);
            estat_lectura = LECTURA_ERROR;
        }
    }

    for (i = 0; estat_lectura == LECTURA_OK && i < config->n_foods; i++) {

        config->foods[i].name = UTILS_readUntil(fd, ' ');

        if (config->foods[i].name == NULL || CONFIG_llegirEnter(fd, '\n', &config->foods[i].amount) < 0) {
            free(config->foods[i].name);
            config->n_foods = i;
            estat_lectura = LECTURA_ERROR;
        }
    }

    close(fd);

    if (estat_lectura == LECTURA_ERROR) {
        if (config->foods == NULL) {
            config->n_foods = 0;
        }
        CONFIG_freeOdysseusConfig(config);
        return -1;
    }

    return 0;
}

void CONFIG_freeOdysseusConfig (OdysseusConfig *config) {
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