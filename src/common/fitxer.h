/***********************************************
*
* @Proposit: Operacions amb fitxers per a les transferencies del sistema.
* @Autor/s: Arnau Ricart i Gerard Mas
* @Data creacio: 14/11/2026
* @Data ultima modificacio: 14/11/2026
*
************************************************/

#ifndef _FITXER_H
#define _FITXER_H

#include "trama.h"

#define FITXER_ERROR -1

#define MSG_FITXER_ERROR_OBRIR "Error: no s'ha pogut obrir el fitxer.\n"
#define MSG_FITXER_ERROR_MIDA "Error: no s'ha pogut obtenir la mida del fitxer.\n"

/***********************************************
*
* @Finalitat: Obte la mida en bytes d'un fitxer.
* @Parametres:  in: path = ruta del fitxer.
* @Retorn: Mida en bytes, o FITXER_ERROR si hi ha un error.
*
************************************************/
int FITXER_midaFitxer (char *path);

#endif