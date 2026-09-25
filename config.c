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

int readIslandConfig (char *nom_fitxer, IslandConfig *config) {
    
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

    aux = readUntil(fd, '\n');
    config->capacity = parseInt(aux);
    free(aux);
    
    aux = readUntil(fd, '\n');
    free(aux);

    config->routes = NULL;
    config->n_routes = 0;
    

    while((aux = readUntil(fd, ' ')) != NULL) {
        config->routes = realloc(config->routes, sizeof(Route) * (config->n_routes + 1));
        
        config->routes[config->n_routes].name = aux;
        config->routes[config->n_routes].ip = readUntil(fd, ' ');

        aux = readUntil(fd, '\n');
        config->routes[config->n_routes].port = parseInt(aux);
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
    int fd = 0;
    *productes = NULL;
    *n_productes = 0;
    Product producte;

    fd = open(nom_fitxer, O_RDONLY);

    if (fd < 0) {
        return -1;
    }

    while(read(fd,&producte,sizeof(Product)) == sizeof(Product)) {
        (*productes) = realloc((*productes), sizeof(Product) * ((*n_productes) + 1));
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

    //Muntem l'estructura que espera SPHRAGIS a partir de la nostra: ell nomes
    //vol el nom de l'illa i un array amb els noms de les destinacions.
    illa.name = config->name;
    illa.known_island_count = config->n_routes;
    illa.known_islands = malloc(sizeof(char *) * config->n_routes);

    for (i = 0; i < config->n_routes; i++) {
        illa.known_islands[i] = config->routes[i].name;
    }

    n_valides = SPHRAGIS_filter_island_configuration(&illa);

    if (n_valides < 0) {
        free(illa.known_islands);
        return n_valides;
    }

    //SPHRAGIS ja ha alliberat els noms de les rutes rebutjades, de manera que
    //nomes podem comparar punters: llegir el contingut d'un nom rebutjat seria
    //accedir a memoria alliberada.
    rutes_valides = malloc(sizeof(Route) * n_valides);
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
            //El name ja l'ha alliberat SPHRAGIS; la ip es nostra.
            free(config->routes[i].ip);
        }
    }

    free(config->routes);
    free(illa.known_islands);

    config->routes = rutes_valides;
    config->n_routes = n_valides;

    return n_valides;
}
