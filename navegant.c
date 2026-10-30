/***********************************************
*
* @Proposit: Implementacio de l'estat del navegant Odysseus.
* @Autor/s: Arnau Ricart i Gerard Mas
* @Data creacio: 29/10/2026
* @Data ultima modificacio: 29/10/2026
*
************************************************/

//Include del fitxer .h del mateix modul
#include "navegant.h"

//Procediments i funcions

int inicialitzarNavegant (Navegant *navegant) {
    navegant->estat = ESTAT_ITHACA;
    navegant->fd_connexio = ERR_CODI;
    navegant->te_viatge = SENSE_VIATGE;
    navegant->viatge.object = NULL;
    navegant->viatge.file = NULL;
    navegant->viatge.destination = NULL;
    navegant->mapa = NULL;
    navegant->n_illes = 0;
    navegant->adreces = NULL;
    navegant->n_adreces = 0;

    navegant->ubicacio = copiarText(TEXT_ITHACA);
    if (navegant->ubicacio == NULL) {
        printF(1, MSG_SENSE_MEMORIA);
        return ERR_CODI;
    }

    return 0;
}

void alliberarNavegant (Navegant *navegant) {
    int i;
    int j;

    if (navegant->fd_connexio != ERR_CODI) {
        close(navegant->fd_connexio);
    }

    free(navegant->ubicacio);
    free(navegant->viatge.object);
    free(navegant->viatge.destination);

    for (i = 0; i < navegant->n_illes; i++) {
        for (j = 0; j < navegant->mapa[i].known_island_count; j++) {
            free(navegant->mapa[i].known_islands[j]);
        }
        free(navegant->mapa[i].known_islands);
        free(navegant->mapa[i].name);
    }
    free(navegant->mapa);

    for (i = 0; i < navegant->n_adreces; i++) {
        free(navegant->adreces[i].name);
        free(navegant->adreces[i].ip);
    }
    free(navegant->adreces);

    freeOdysseusConfig(&navegant->config);
}

int totalProvisions (Navegant *navegant) {
    int i;
    int total = 0;

    for (i = 0; i < navegant->config.n_foods; i++) {
        total = total + navegant->config.foods[i].amount;
    }

    return total;
}


void consumirRacio (Navegant *navegant, int quilos) {
    int i = 0;
    int pendent = quilos;

    while (pendent > 0 && i < navegant->config.n_foods) {
        if (navegant->config.foods[i].amount >= pendent) {
            navegant->config.foods[i].amount = navegant->config.foods[i].amount - pendent;
            pendent = 0;
        } else {
            pendent = pendent - navegant->config.foods[i].amount;
            navegant->config.foods[i].amount = 0;
        }
        i++;
    }
}

int buscarAdreca (Navegant *navegant, char *nom) {
    int i;
    int index = ERR_CODI;

    for (i = 0; i < navegant->n_adreces; i++) {
        if (strcasecmp(navegant->adreces[i].name, nom) == 0) {
            index = i;
        }
    }

    return index;
}

int esRutaDirecta (Navegant *navegant, char *desti) {
    int i;
    int j;
    int directa = RUTA_NO_DIRECTA;

    for (i = 0; i < navegant->n_illes; i++) {
        if (strcasecmp(navegant->mapa[i].name, navegant->ubicacio) == 0) {
            for (j = 0; j < navegant->mapa[i].known_island_count; j++) {
                if (strcasecmp(navegant->mapa[i].known_islands[j], desti) == 0) {
                    directa = RUTA_DIRECTA;
                }
            }
        }
    }

    return directa;
}

int canviarUbicacio (Navegant *navegant, char *nom) {
    char *nova = NULL;
    int resultat = ERR_CODI;

    nova = copiarText(nom);

    if (nova == NULL) {
        printF(1, MSG_SENSE_MEMORIA);
    } else {
        free(navegant->ubicacio);
        navegant->ubicacio = nova;
        resultat = 0;
    }

    return resultat;
}
