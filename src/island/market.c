/***********************************************
*
* @Proposit: Implementacio del mercat de l'illa, llistat, compra i venda de
*            productes i venda del mapa local.
* @Autor/s: Arnau Ricart i Gerard Mas
* @Data creacio: 29/10/2026
* @Data ultima modificacio: 29/10/2026
*
************************************************/

//Include del fitxer .h del mateix modul
#include "market.h"

//Procediments i funcions

void MARKET_inicialitzarMercat (Mercat *mercat) {
    pthread_mutex_init(&mercat->mutex_stock, NULL);
}

void MARKET_destruirMercat (Mercat *mercat) {
    pthread_mutex_destroy(&mercat->mutex_stock);
}

int MARKET_guardarStock (Mercat *mercat) {
    int fd;
    int i;
    int resultat = 0;

    fd = open(mercat->fitxer, O_WRONLY | O_TRUNC);
    if (fd < 0) {
        return ERR_CODI;
    }

    for (i = 0; i < mercat->n_productes; i++) {
        if (write(fd, &mercat->productes[i], sizeof(Product)) != sizeof(Product)) {
            resultat = ERR_CODI;
        }
    }

    close(fd);

    return resultat;
}

int MARKET_buscarProducte (Mercat *mercat, char *nom) {
    int i;
    int index = ERR_CODI;

    for (i = 0; i < mercat->n_productes; i++) {
        if (strcasecmp(mercat->productes[i].name, nom) == 0) {
            index = i;
        }
    }

    return index;
}

void MARKET_atendreListMarket (int fd, Mercat *mercat) {
    char *text = NULL;
    int total;
    int i;

    total = mercat->n_productes + 1;

    for (i = 0; i < mercat->n_productes; i++) {
        asprintf(&text, FORMAT_PRODUCTE, i + 1, total, mercat->productes[i].name, mercat->productes[i].amount, mercat->productes[i].price);
        TRAMA_enviarTrama(fd, TIPUS_LIST_MARKET, FLAG_OK, text);
        free(text);
    }

    asprintf(&text, FORMAT_MAPA_LLISTA, total, total, PREU_MAPA);
    TRAMA_enviarTrama(fd, TIPUS_LIST_MARKET, FLAG_OK, text);
    free(text);
}

int MARKET_validarPeticio (int fd, int tipus, char *dades, Mercat *mercat, int *index, int *quantitat) {
    char **trossos = NULL;
    int n_trossos = 0;
    int valida = PETICIO_NO_VALIDA;

    trossos = UTILS_splitString(dades, '&', &n_trossos);

    if (n_trossos != 2 || UTILS_esNumeric(trossos[1]) == TEXT_NO_NUMERIC || atoi(trossos[1]) <= 0) {
        TRAMA_enviarTrama(fd, tipus, FLAG_ERROR, TEXT_ERROR_PETICIO);
    } else {
        *index = MARKET_buscarProducte(mercat, trossos[0]);
        *quantitat = atoi(trossos[1]);

        if (*index == ERR_CODI) {
            TRAMA_enviarTrama(fd, tipus, FLAG_ERROR, TEXT_ERROR_PRODUCTE);
        } else {
            valida = PETICIO_VALIDA;
        }
    }

    UTILS_freeTokens(trossos, n_trossos);

    return valida;
}

void MARKET_atendreBuy (int fd, char *nom_vaixell, char *dades, Mercat *mercat) {
    Product *producte = NULL;
    char *text = NULL;
    int index;
    int quantitat;
    int cost;

    if (MARKET_validarPeticio(fd, TIPUS_BUY, dades, mercat, &index, &quantitat) == PETICIO_NO_VALIDA) {
        return;
    }

    producte = &mercat->productes[index];

    if (quantitat > producte->amount) {
        TRAMA_enviarTrama(fd, TIPUS_BUY, FLAG_ERROR, TEXT_ERROR_STOCK);
        return;
    }

    producte->amount = producte->amount - quantitat;
    cost = quantitat * producte->price;
    MARKET_guardarStock(mercat);

    asprintf(&text, FORMAT_OPERACIO, producte->name, quantitat, cost);
    TRAMA_enviarTrama(fd, TIPUS_BUY, FLAG_OK, text);
    free(text);

    asprintf(&text, MSG_COMPRA, nom_vaixell, quantitat, producte->name);
    UTILS_printF(1, text);
    free(text);
}

void MARKET_atendreSell (int fd, char *nom_vaixell, char *dades, Mercat *mercat) {
    Product *producte = NULL;
    char *text = NULL;
    int index;
    int quantitat;
    int ingressos;

    if (MARKET_validarPeticio(fd, TIPUS_SELL, dades, mercat, &index, &quantitat) == PETICIO_NO_VALIDA) {
        return;
    }

    producte = &mercat->productes[index];

    producte->amount = producte->amount + quantitat;
    ingressos = quantitat * producte->price;
    MARKET_guardarStock(mercat);

    asprintf(&text, FORMAT_OPERACIO, producte->name, quantitat, ingressos);
    TRAMA_enviarTrama(fd, TIPUS_SELL, FLAG_OK, text);
    free(text);

    asprintf(&text, MSG_VENDA, nom_vaixell, quantitat, producte->name);
    UTILS_printF(1, text);
    free(text);
}

void MARKET_atendreBuyMap (int fd, char *nom_vaixell, char *dades, IslandConfig *config) {
    char *text = NULL;
    int i;

    if (UTILS_esNumeric(dades) == TEXT_NO_NUMERIC || atoi(dades) != 1) {
        TRAMA_enviarTrama(fd, TIPUS_BUY_MAP, FLAG_ERROR, TEXT_ERROR_PETICIO);
        return;
    }

    if (config->n_routes == 0) {
        asprintf(&text, FORMAT_SENSE_RUTES, PREU_MAPA);
        TRAMA_enviarTrama(fd, TIPUS_BUY_MAP, FLAG_OK, text);
        free(text);
    }

    for (i = 0; i < config->n_routes; i++) {
        asprintf(&text, FORMAT_RUTA, PREU_MAPA, i + 1, config->n_routes, config->routes[i].name, config->routes[i].ip, config->routes[i].port);
        TRAMA_enviarTrama(fd, TIPUS_BUY_MAP, FLAG_OK, text);
        free(text);
    }

    asprintf(&text, MSG_MAPA, nom_vaixell);
    UTILS_printF(1, text);
    free(text);
}
