/***********************************************
*
* @Proposit: Estat del navegant Odysseus: diners, carrega, ubicacio, viatge
*            actual i mapa conegut.
* @Autor/s: Arnau Ricart i Gerard Mas
* @Data creacio: 29/10/2026
* @Data ultima modificacio: 29/10/2026
*
************************************************/

//Define Guards
#ifndef _NAVEGANT_H
#define _NAVEGANT_H

//Llibreries
#include <strings.h>

//Llibreries propies
#include "trama.h"
#include "config.h"
#include "sphragis.h"

//Constants
#define ESTAT_ITHACA           0
#define ESTAT_CONNECTAT_ITHACA 1
#define ESTAT_ATRACAT          2
#define ESTAT_DESCONNECTAT     3
#define SENSE_VIATGE 0
#define AMB_VIATGE   1
#define RUTA_NO_DIRECTA 0
#define RUTA_DIRECTA    1

#define TEXT_ITHACA "Ithaca"
#define TEXT_AEAEA  "Aeaea"

//Structs
typedef struct {
    OdysseusConfig config;
    int estat;
    int fd_connexio;
    char *ubicacio;
    int te_viatge;
    Voyage viatge;
    SPHRAGIS_Island *mapa;
    int n_illes;
    Route *adreces;
    int n_adreces;
} Navegant;

//Procediments i funcions

/***********************************************
*
* @Finalitat: Deixa el navegant a Ithaca, sense connexio, sense viatge i
*            sense mapa. La configuracio ja ha d'estar llegida.
* @Parametres:  in/out: navegant = navegant a inicialitzar.
* @Retorn: 0 si s'ha inicialitzat, ERR_CODI si no hi ha memoria.
*
************************************************/
int inicialitzarNavegant (Navegant *navegant);

/***********************************************
*
* @Finalitat: Tanca la connexio activa i allibera tota la memoria del navegant.
* @Parametres:  in/out: navegant = navegant a alliberar.
* @Retorn: ----.
*
************************************************/
void alliberarNavegant (Navegant *navegant);

/***********************************************
*
* @Finalitat: Calcula el total de provisions, la suma dels quilograms de
*             tota la carrega.
* @Parametres:  in: navegant = navegant amb la carrega.
* @Retorn: Total de quilograms de menjar.
*
************************************************/
int totalProvisions (Navegant *navegant);

/***********************************************
*
* @Finalitat: Resta quilograms de la carrega, comencant pel primer aliment.
* @Parametres:  in/out: navegant = navegant amb la carrega.
*               in: quilos = quilograms a consumir.
* @Retorn: ----.
*
************************************************/
void consumirRacio (Navegant *navegant, int quilos);

/***********************************************
*
* @Finalitat: Busca l'adreca coneguda d'una illa pel seu nom.
* @Parametres:  in: navegant = navegant amb les adreces conegudes.
*               in: nom = nom de l'illa.
* @Retorn: Posicio de l'adreca, o ERR_CODI si no es coneix.
*
************************************************/
int buscarAdreca (Navegant *navegant, char *nom);

/***********************************************
*
* @Finalitat: Comprova si al mapa conegut hi ha una ruta directa des de la
*             ubicacio actual fins a una illa.
* @Parametres:  in: navegant = navegant amb el mapa i la ubicacio.
*               in: desti = nom de l'illa de desti.
* @Retorn: RUTA_DIRECTA o RUTA_NO_DIRECTA.
*
************************************************/
int esRutaDirecta (Navegant *navegant, char *desti);

/***********************************************
*
* @Finalitat: Canvia la ubicacio actual del navegant.
* @Parametres:  in/out: navegant = navegant a actualitzar.
*               in: nom = nom de la nova ubicacio.
* @Retorn: 0 si s'ha canviat, ERR_CODI si no hi ha memoria.
*
************************************************/
int canviarUbicacio (Navegant *navegant, char *nom);

#endif