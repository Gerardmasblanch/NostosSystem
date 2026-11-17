/***********************************************
*
* @Proposit: Lectura dels fitxers de configuracio del sistema.
* @Autor/s: Arnau Ricart i Gerard Mas
* @Data creacio: 24/09/2026
* @Data ultima modificacio: 17/11/2026
*
************************************************/

//Define Guards
#ifndef _CONFIG_H
#define _CONFIG_H

//Llibreries propies
#include "utils.h"
#include "types.h"
#include "sphragis.h"

//Constants
#define LECTURA_OK    0
#define LECTURA_ERROR 1
#define LECTURA_FINAL 2

//Procediments i funcions

/***********************************************
*
* @Finalitat: Llegeix un camp numeric d'un fitxer fins al delimitador.
* @Parametres:  in: fd = fitxer d'on es llegeix.
*               in: delimitador = caracter que acaba el camp.
*               out: valor = enter llegit.
* @Retorn: 0 si va be, -1 si el fitxer s'ha acabat abans del camp.
*
************************************************/
int CONFIG_llegirEnter (int fd, char delimitador, int *valor);

/***********************************************
*
* @Finalitat: Llegeix el fitxer de configuracio d'Ithaca.
* @Parametres:  in: nom_fitxer = ruta del fitxer.
*               out: config = configuracio llegida.
* @Retorn: 0 si va be, -1 si no s'ha pogut obrir el fitxer o li falta algun camp.
*
************************************************/
int CONFIG_readIthacaConfig (char *nom_fitxer, IthacaConfig *config);

/***********************************************
*
* @Finalitat: Allibera la configuracio d'Ithaca.
* @Parametres:  in/out: config = configuracio a alliberar.
* @Retorn: ----.
*
************************************************/
void CONFIG_freeIthacaConfig (IthacaConfig *config);

/***********************************************
*
* @Finalitat: Llegeix el fitxer de viatges d'Ithaca i els hi assigna un
*             identificador, que no ve al fitxer.
* @Parametres:  in: nom_fitxer = ruta del fitxer.
*               out: viatges = array de viatges llegits.
*               out: n_viatges = nombre de viatges.
* @Retorn: 0 si va be, -1 si no s'ha pogut obrir el fitxer o li falta algun camp.
*
************************************************/
int CONFIG_readVoyages (char *nom_fitxer, Voyage **viatges, int *n_viatges);

/***********************************************
*
* @Finalitat: Allibera l'array de viatges.
* @Parametres:  in/out: viatges = array a alliberar.
*               in: n_viatges = nombre de viatges.
* @Retorn: ----.
*
************************************************/
void CONFIG_freeVoyages (Voyage *viatges, int n_viatges);

/***********************************************
*
* @Finalitat: Llegeix el fitxer de configuracio d'una illa i les seves rutes.
* @Parametres:  in: nom_fitxer = ruta del fitxer.
*               out: config = configuracio llegida.
* @Retorn: 0 si va be, -1 si no s'ha pogut obrir el fitxer o li falta algun camp.
*
************************************************/
int CONFIG_readIslandConfig (char *nom_fitxer, IslandConfig *config);

/***********************************************
*
* @Finalitat: Allibera la configuracio d'una illa i les seves rutes.
* @Parametres:  in/out: config = configuracio a alliberar.
* @Retorn: ----.
*
************************************************/
void CONFIG_freeIslandConfig (IslandConfig *config);

/***********************************************
*
* @Finalitat: Carrega a memoria el fitxer binari de stock d'una illa.
* @Parametres:  in: nom_fitxer = ruta del fitxer.
*               out: productes = array de productes llegits.
*               out: n_productes = nombre de productes.
* @Retorn: 0 si va be, -1 si no s'ha pogut obrir el fitxer.
*
************************************************/
int CONFIG_readStock (char *nom_fitxer, Product **productes, int *n_productes);

/***********************************************
*
* @Finalitat: Allibera l'array de productes.
* @Parametres:  in/out: productes = array a alliberar.
* @Retorn: ----.
*
************************************************/
void CONFIG_freeStock (Product *productes);

/***********************************************
*
* @Finalitat: Valida les rutes d'una illa amb SPHRAGIS, descartant les
*             destinacions que no hi estan realment connectades.
* @Parametres:  in/out: config = configuracio de l'illa.
* @Retorn: El nombre de rutes valides, o un codi d'error de SPHRAGIS.
*
************************************************/
int CONFIG_filterIslandRoutes (IslandConfig *config);

/***********************************************
*
* @Finalitat: Llegeix el fitxer de configuracio d'un Odysseus.
* @Parametres:  in: nom_fitxer = ruta del fitxer.
*               out: config = configuracio llegida.
* @Retorn: 0 si va be, -1 si no s'ha pogut obrir el fitxer o li falta algun camp.
*
************************************************/
int CONFIG_readOdysseusConfig (char *nom_fitxer, OdysseusConfig *config);

/***********************************************
*
* @Finalitat: Allibera la configuracio d'un Odysseus i els seus aliments.
* @Parametres:  in/out: config = configuracio a alliberar.
* @Retorn: ----.
*
************************************************/
void CONFIG_freeOdysseusConfig (OdysseusConfig *config);

#endif
