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

#include <sys/wait.h>

#include "trama.h"

#define FITXER_ERROR -1


#define MIDA_MD5 32
#define FITXER_OK 0
#define COMANDA_MD5 "md5sum"

#define MSG_FITXER_ERROR_MD5 "Error: no s'ha pogut calcular l'MD5 del fitxer.\n"
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


/***********************************************
*
* @Finalitat: Executa md5sum i obte el resum MD5 d'un fitxer.
* @Parametres:  in: path = ruta del fitxer.
*               out: md5 = buffer amb espai per a MIDA_MD5 + 1 caracters.
* @Retorn: FITXER_OK si s'ha obtingut l'MD5, FITXER_ERROR si no.
*
************************************************/
int FITXER_calcularMD5 (char *path, char *md5);

#endif