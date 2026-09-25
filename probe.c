/***********************************************
*
* @Proposit: Programa auxiliar de proves. Per a cada illa de l'arxipelag,
*            proposa a SPHRAGIS totes les altres com a destinacions i mostra
*            quines superen el filtratge, de manera que es pugui descobrir la
*            topologia real que la llibreria te definida.
*            NO forma part de la practica: no s'ha d'entregar.
* @Autor/s: Arnau Ricart i Gerard Mas
* @Data creacio: 25/09/2026
* @Data ultima modificacio: 25/09/2026
*
************************************************/

//Llibreries del sistema
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Llibreries propies
#include "sphragis.h"

#define N_ILLES 6

int main (void) {
    char *noms[N_ILLES] = {"Aeaea", "Ogygia", "Scheria", "Thrinacia", "Aeolia", "Ismarus"};
    SPHRAGIS_Island illa;
    int n_valides;
    int i;
    int j;
    int k;

    printf("Topologia real segons SPHRAGIS:\n\n");

    for (i = 0; i < N_ILLES; i++) {
        //Proposem totes les altres illes com a destinacions d'aquesta.
        illa.name = noms[i];
        illa.known_island_count = N_ILLES - 1;
        illa.known_islands = malloc(sizeof(char *) * (N_ILLES - 1));

        k = 0;
        for (j = 0; j < N_ILLES; j++) {
            if (j != i) {
                illa.known_islands[k] = strdup(noms[j]);
                k++;
            }
        }

        n_valides = SPHRAGIS_filter_island_configuration(&illa);

        printf("%-10s -> ", noms[i]);

        if (n_valides < 0) {
            printf("ERROR %d\n", n_valides);
        } else {
            for (k = 0; k < n_valides; k++) {
                printf("%s ", illa.known_islands[k]);
                free(illa.known_islands[k]);
            }
            printf("  (%d rutes)\n", n_valides);
        }

        free(illa.known_islands);
    }

    return 0;
}
