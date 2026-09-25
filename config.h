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
#include "sphragis.h"

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

/***
*
* @Finalitat: Llegeix el fitxer de configuracio de les rutes d'una il·lota i omple
*             l'estructura corresponent.
* @Parametres:  in: nom_fitxer = ruta del fitxer de configuracio.
*               out: config = estructura on es guarda la configuracio llegida.
* @Retorn: 0 si s'ha llegit correctament, -1 si no s'ha pogut obrir el fitxer.
*
************************************************/
int readIslandConfig (char *nom_fitxer, IslandConfig *config);

/***********************************************
* 
* @Finalitat: Allibera l'espai de memòria ocupat per la configuració d'una il·lota.
* @Parametres:  in/out: config = estructura de configuració de l'il·lota.
* @Retorn: ----.
*
************************************************/

void freeIslandConfig (IslandConfig *config);

/***********************************************
*
* @Finalitat: Allibera l'espai de memòria ocupat per l'array de productes.
* @Parametres:  in/out: productes = array d'estructures de productes.
*               in: n_productes = nombre de productes.
* @Retorn: ----.
*
************************************************/

int readStock(char *nom_fitxer, Product **productes, int *n_productes);

/***********************************************
* @Finalitat: Allibera l'espai de memòria ocupat per l'array de productes.
* @Parametres:  in/out: productes = array d'estructures de productes.
*               in: n_productes = nombre de productes.
* @Retorn: ----.
*
************************************************/
void freeStock(Product *productes);

/***********************************************
*
* @Finalitat: Validar les rutes carregades d'una illa amb la llibreria
*             SPHRAGIS, eliminant de la configuracio les destinacions que no
*             hi estiguin realment connectades.
* @Parametres:  in/out: config = configuracio de l'illa. A la sortida nomes
*               conte les rutes que han superat el filtratge.
* @Retorn: El nombre de rutes valides, o un codi d'error negatiu de SPHRAGIS
*          (SPHRAGIS_ERROR_INVALID_ISLAND o SPHRAGIS_ERROR_INVALID_CONNECTION).
*
************************************************/
int filterIslandRoutes (IslandConfig *config);

/***********************************************
*
* @Finalitat: Llegeix el fitxer de configuracio d'un proces Odysseus i omple
*             l'estructura corresponent, incloent-hi la llista d'aliments.
* @Parametres:  in: nom_fitxer = ruta del fitxer de configuracio.
*               out: config = estructura on es guarda la configuracio llegida.
* @Retorn: 0 si s'ha llegit correctament, -1 si no s'ha pogut obrir el fitxer.
*
************************************************/
int readOdysseusConfig (char *nom_fitxer, OdysseusConfig *config);

/***********************************************
*
* @Finalitat: Allibera la memoria dinamica de l'estructura de configuracio
*             d'un proces Odysseus, incloent-hi la llista d'aliments.
* @Parametres:  in/out: config = estructura que es vol alliberar.
* @Retorn: ----.
*
************************************************/
void freeOdysseusConfig (OdysseusConfig *config);

#endif