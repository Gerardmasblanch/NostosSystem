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