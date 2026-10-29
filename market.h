/***********************************************
*
* @Proposit: Mercat de l'illa: llistat, compra i venda de productes i venda
*            del mapa local.
* @Autor/s: Arnau Ricart i Gerard Mas
* @Data creacio: 29/10/2026
* @Data ultima modificacio: 29/10/2026
*
************************************************/

//Define Guards
#ifndef _MARKET_H
#define _MARKET_H

//Llibreries propies
#include "trama.h"
#include "config.h"

//Constants
#define PREU_MAPA 50

#define FORMAT_PRODUCTE   "%d&%d&%s&%d&%d"
#define FORMAT_MAPA_LLISTA "%d&%d&MAP&1&%d"
#define PETICIO_NO_VALIDA 0
#define PETICIO_VALIDA    1
#define TEXT_ERROR_PETICIO  "INVALID_DATA"
#define TEXT_ERROR_PRODUCTE "UNKNOWN_PRODUCT"
#define TEXT_ERROR_STOCK    "NOT_ENOUGH_STOCK"
#define FORMAT_OPERACIO "OK&%s&%d&%d"
#define MSG_COMPRA ">>> %s bought %d kg of %s.\n"
#define MSG_VENDA  ">>> %s sold %d kg of %s.\n"
#define FORMAT_RUTA       "OK&%d&%d&%d&%s&%s&%d"
#define FORMAT_SENSE_RUTES "OK&%d&0"
#define MSG_MAPA ">>> %s bought the local map.\n"


//Tipus propis
typedef struct {
    Product *productes;
    int n_productes;
    char *fitxer;
} Mercat;

//Procediments i funcions

/***********************************************
*
* @Finalitat: Sobreescriu el fitxer de stock amb l'estat actual del mercat.
* @Parametres:  in: mercat = mercat a guardar.
* @Retorn: 0 si s'ha guardat, ERR_CODI si hi ha hagut un error.
*
************************************************/
int guardarStock (Mercat *mercat);

/***********************************************
*
* @Finalitat: Busca un producte del mercat pel seu nom.
* @Parametres:  in: mercat = mercat on es busca.
*               in: nom = nom del producte.
* @Retorn: Posicio del producte a la llista, o ERR_CODI si no hi es.
*
************************************************/
int buscarProducte (Mercat *mercat, char *nom);

/***********************************************
*
* @Finalitat: Envia el contingut del mercat, una trama per producte i una
*             darrera per al mapa.
* @Parametres:  in: fd = socket del vaixell.
*               in: mercat = mercat de l'illa.
* @Retorn: ----.
*
************************************************/
void atendreListMarket (int fd, Mercat *mercat);

/***********************************************
*
* @Finalitat: Valida una peticio de compra o venda <Product>&<Amount>. Si no
*             es valida, envia l'error al vaixell.
* @Parametres:  in: fd = socket del vaixell.
*               in: tipus = tipus de la trama (BUY o SELL) per a la resposta.
*               in: dades = text de la peticio.
*               in: mercat = mercat de l'illa.
*               out: index = posicio del producte al mercat.
*               out: quantitat = quilograms demanats.
* @Retorn: PETICIO_VALIDA o PETICIO_NO_VALIDA.
*
************************************************/
int validarPeticio (int fd, int tipus, char *dades, Mercat *mercat, int *index, int *quantitat);

/***********************************************
*
* @Finalitat: Atén una compra: resta el stock, el guarda i respon amb el cost.
* @Parametres:  in: fd = socket del vaixell.
*               in: nom_vaixell = nom del vaixell que compra.
*               in: dades = text de la peticio.
*               in/out: mercat = mercat de l'illa.
* @Retorn: ----.
*
************************************************/
void atendreBuy (int fd, char *nom_vaixell, char *dades, Mercat *mercat);

/***********************************************
*
* @Finalitat: Atén una venda: suma el stock, el guarda i respon amb els
*             ingressos.
* @Parametres:  in: fd = socket del vaixell.
*               in: nom_vaixell = nom del vaixell que ven.
*               in: dades = text de la peticio.
*               in/out: mercat = mercat de l'illa.
* @Retorn: ----.
*
************************************************/
void atendreSell (int fd, char *nom_vaixell, char *dades, Mercat *mercat);

/***********************************************
*
* @Finalitat: Atén la compra del mapa local: envia una trama per cada ruta
*             de l'illa.
* @Parametres:  in: fd = socket del vaixell.
*               in: nom_vaixell = nom del vaixell que compra.
*               in: dades = quantitat demanada (nomes es valid 1).
*               in: config = configuracio de l'illa amb les rutes.
* @Retorn: ----.
*
************************************************/
void atendreBuyMap (int fd, char *nom_vaixell, char *dades, IslandConfig *config);

#endif