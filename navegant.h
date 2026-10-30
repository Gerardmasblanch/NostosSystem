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

#define MSG_CARREGA "\nCargo:\n"
#define FORMAT_ALIMENT "%s: %d kg\n"
#define FORMAT_TOTAL_PROVISIONS "Total provisions: %d kg\n"

#define FORMAT_CAPCALERA_STATUS "\n--- %s ---\nGold: %d\n"
#define FORMAT_VIATGE_STATUS "\nCurrent voyage: %s -> %s\nReward: %d\n"
#define MSG_STATUS_SENSE_VIATGE "\nCurrent voyage: None\n"
#define FORMAT_UBICACIO_STATUS "Current location: %s\n"

#define TEXT_COMPRA_OK "OK"
#define TEXT_VENDA_OK "OK"

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

/***********************************************
*
* @Finalitat: Mostrar els aliments transportats i el total de provisions.
* @Parametres: in: navegant = navegant amb la carrega actual.
* @Retorn: ----.
*
************************************************/
void mostrarCarrega (Navegant *navegant);

/***********************************************
*
* @Finalitat: Mostrar els recursos, el viatge i la ubicacio del navegant.
* @Parametres: in: navegant = navegant amb l'estat actual.
* @Retorn: ----.
*
************************************************/
void mostrarStatus (Navegant *navegant);

/***********************************************
*
* @Finalitat: Buscar un aliment pel nom dins de la carrega.
* @Parametres: in: navegant = navegant amb la carrega actual.
*              in: nom = nom de l'aliment que es busca.
* @Retorn: Posicio de l'aliment, o ERR_CODI si no existeix.
*
************************************************/
int buscarAliment (Navegant *navegant, char *nom);

/***********************************************
*
* @Finalitat: Obtenir la posicio d'un aliment, creant-lo a zero si es nou.
* @Parametres: in/out: navegant = navegant amb la carrega actual.
*              in: nom = nom de l'aliment que es prepara.
* @Retorn: Posicio de l'aliment, o ERR_CODI si falta memoria.
*
************************************************/
int prepararAliment (Navegant *navegant, char *nom);

/***********************************************
*
* @Finalitat: Validar la confirmacio d'una compra i actualitzar els recursos.
* @Parametres: in/out: navegant = navegant amb la carrega i els diners.
*              in: index = posicio de l'aliment preparada abans de comprar.
*              in: quantitat = quilograms demanats a la compra.
*              in: dades = resposta OK&Product&Amount&Cost.
*              out: cost = cost confirmat, nomes valid si retorna TRAMA_OK.
* @Retorn: TRAMA_OK si s'ha aplicat, TRAMA_ERROR si la resposta es incorrecta.
*
************************************************/
int guardarCompra (Navegant *navegant, int index, int quantitat, char *dades, int *cost);

/***********************************************
*
* @Finalitat: Validar la confirmacio d'una venda i actualitzar els recursos.
* @Parametres: in/out: navegant = navegant amb la carrega i els diners.
*              in: index = posicio de l'aliment que es ven.
*              in: quantitat = quilograms demanats a la venda.
*              in: dades = resposta OK&Product&Amount&Income.
*              out: ingressos = import confirmat, valid si retorna TRAMA_OK.
* @Retorn: TRAMA_OK si s'ha aplicat, TRAMA_ERROR si la resposta es incorrecta.
*
************************************************/
int guardarVenda (Navegant *navegant, int index, int quantitat, char *dades, int *ingressos);

#endif