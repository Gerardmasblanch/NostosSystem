/***********************************************
*
* @Proposit: Lectura dels fitxers de configuracio dels tres processos
*            del sistema.
* @Autor/s: Arnau Ricart i Gerard Mas
* @Data creacio: 24/09/2026
* @Data ultima modificacio: 24/09/2026
*
************************************************/

//Define Guards
#ifndef _CONFIG_H
#define _CONFIG_H

//Llibreries propies
#include "utils.h"
#include "types.h"

//Procediments i funcions

/***********************************************
*
* @Finalitat: Llegeix el fitxer de configuracio del proces Ithaca i omple
*             l'estructura corresponent.
* @Parametres:  in: nom_fitxer = ruta del fitxer de configuracio.
*               out: config = estructura on es guarda la configuracio llegida.
* @Retorn: 0 si s'ha llegit correctament, -1 si no s'ha pogut obrir el fitxer.
*
************************************************/
int readIthacaConfig (char *nom_fitxer, IthacaConfig *config);

/***********************************************
*
* @Finalitat: free de la configuració del proces Ithaca.
* @Parametres:  in/out: config = estructura que es vol alliberar.
* @Retorn: ----.
*
************************************************/
void freeIthacaConfig (IthacaConfig *config);

/***********************************************
*
* @Finalitat: Llegeix el fitxer de configuracio dels viatges i omple
*             l'estructura corresponent.
* @Parametres:  in: nom_fitxer = ruta del fitxer de configuracio.
*               out: viatges = array d'estructures de viatge.
*               out: n_viatges = nombre de viatges llegits.
* @Retorn: 0 si s'ha llegit correctament, -1 si no s'ha pogut obrir el fitxer.
*
************************************************/
int readVoyages(char *nom_fitxer, Voyage **viatges, int *n_viatges);

/***********************************************
*
* @Finalitat: Allibera l'espai de memòria ocupat per l'array de viatges.
* @Parametres:  in/out: viatges = array d'estructures de viatge.
*               in: n_viatges = nombre de viatges.
* @Retorn: ----.
*
************************************************/
void freeVoyages(Voyage *viatges, int n_viatges);

#endif