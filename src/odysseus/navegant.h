/***********************************************
*
* @Proposit: Estat del navegant Odysseus, diners, carrega, ubicacio, viatge
*            actual i mapa conegut.
* @Autor/s: Arnau Ricart i Gerard Mas
* @Data creacio: 29/10/2026
* @Data ultima modificacio: 17/11/2026
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
#include "fitxer.h"
#include "sphragis.h"

//Constants
#define ESTAT_ITHACA           0
#define ESTAT_CONNECTAT_ITHACA 1
#define ESTAT_ATRACAT          2
#define ESTAT_DESCONNECTAT     3
#define ESTAT_ESPERANT         4
#define ESTAT_PERDUT           5
#define SENSE_VIATGE 0
#define AMB_VIATGE   1
#define VIATGE_NO_LLIURAT 0
#define VIATGE_LLIURAT    1
#define RUTA_NO_DIRECTA 0
#define RUTA_DIRECTA    1

#define TEXT_ITHACA "Ithaca"
#define TEXT_AEAEA  "Aeaea"

#define MSG_CARREGA "\nCargo:\n"
#define FORMAT_ALIMENT "%s: %d kg\n"
#define FORMAT_TOTAL_PROVISIONS "Total provisions: %d kg\n"

#define FORMAT_CAPCALERA_STATUS "\n--- %s ---\nGold: %d\n"
#define FORMAT_VIATGE_STATUS "\nCurrent voyage: %s -> %s\nReward: %d\n"
#define FORMAT_OBJECTE_STATUS "Object: %s\n"
#define MSG_STATUS_SENSE_VIATGE "\nCurrent voyage: None\n"
#define FORMAT_UBICACIO_STATUS "Current location: %s\n"
#define MSG_UBICACIO_DESCONEGUDA "Current location: None\n"
#define FORMAT_UBICACIO_ESPERANT "Current location: Off the coast of %s\n"


#define TEXT_PREFIX_SEGELLAT "signed_"

#define AMB_SORTIDA      0
#define SENSE_SORTIDA    1
#define OR_MINIM_SORTIDA 50

#define FORMAT_RUTA_ESTAT       "%s/estat.db"
#define FORMAT_ESTAT_RECURSOS   "%d&%d\n"
#define FORMAT_ESTAT_ALIMENT    "%s&%d\n"
#define FORMAT_ESTAT_VIATGE     "%d&%d&%s&%s&%s&%d&%d\n"
#define TEXT_ESTAT_SENSE_VIATGE "0\n"
#define N_CAMPS_ESTAT_RECURSOS  2
#define N_CAMPS_ESTAT_ALIMENT   2
#define N_CAMPS_ESTAT_VIATGE    7
#define MSG_ERROR_GUARDAR_ESTAT "Error: the state could not be saved.\n"

#define TEXT_COMPRA_OK "OK"
#define TEXT_VENDA_OK "OK"

//Structs
typedef struct {
    OdysseusConfig config;
    int estat;
    int fd_connexio;
    int fd_avis;
    char *ubicacio;
    int te_viatge;
    Voyage viatge;
    int viatge_lliurat;
    char *fitxer_segellat;
    SPHRAGIS_Island *mapa;
    int n_illes;
    Route *adreces;
    int n_adreces;
    int mercat_consultat;
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
int NAVEGANT_inicialitzarNavegant (Navegant *navegant);

/***********************************************
*
* @Finalitat: Tanca la connexio activa i allibera tota la memoria del navegant.
* @Parametres:  in/out: navegant = navegant a alliberar.
* @Retorn: ----.
*
************************************************/
void NAVEGANT_alliberarNavegant (Navegant *navegant);

/***********************************************
*
* @Finalitat: Allibera les dades del viatge actual (objecte, desti, ruta del fitxer i ruta de la imatge segellada) i deixa el navegant sense viatge i sense lliurament.
* @Parametres:  in/out: navegant = navegant amb el viatge a esborrar.
* @Retorn: ----.
*
************************************************/
void NAVEGANT_esborrarViatge (Navegant *navegant);

/***********************************************
*
* @Finalitat: Calcula el total de provisions, la suma dels quilograms de
*             tota la carrega.
* @Parametres:  in: navegant = navegant amb la carrega.
* @Retorn: Total de quilograms de menjar.
*
************************************************/
int NAVEGANT_totalProvisions (Navegant *navegant);

/***********************************************
*
* @Finalitat: Resta quilograms de la carrega, comencant pel primer aliment.
* @Parametres:  in/out: navegant = navegant amb la carrega.
*               in: quilos = quilograms a consumir.
* @Retorn: ----.
*
************************************************/
void NAVEGANT_consumirRacio (Navegant *navegant, int quilos);

/***********************************************
*
* @Finalitat: Busca l'adreca coneguda d'una illa pel seu nom.
* @Parametres:  in: navegant = navegant amb les adreces conegudes.
*               in: nom = nom de l'illa.
* @Retorn: Posicio de l'adreca, o ERR_CODI si no es coneix.
*
************************************************/
int NAVEGANT_buscarAdreca (Navegant *navegant, char *nom);

/***********************************************
*
* @Finalitat: Comprova si al mapa conegut hi ha una ruta directa des de la
*             ubicacio actual fins a una illa.
* @Parametres:  in: navegant = navegant amb el mapa i la ubicacio.
*               in: desti = nom de l'illa de desti.
* @Retorn: RUTA_DIRECTA o RUTA_NO_DIRECTA.
*
************************************************/
int NAVEGANT_esRutaDirecta (Navegant *navegant, char *desti);

/***********************************************
*
* @Finalitat: Canvia la ubicacio actual del navegant.
* @Parametres:  in/out: navegant = navegant a actualitzar.
*               in: nom = nom de la nova ubicacio.
* @Retorn: 0 si s'ha canviat, ERR_CODI si no hi ha memoria.
*
************************************************/
int NAVEGANT_canviarUbicacio (Navegant *navegant, char *nom);

/***********************************************
*
* @Finalitat: Mostrar els aliments transportats i el total de provisions.
* @Parametres: in: navegant = navegant amb la carrega actual.
* @Retorn: ----.
*
************************************************/
void NAVEGANT_mostrarCarrega (Navegant *navegant);

/***********************************************
*
* @Finalitat: Mostrar el viatge actual amb la recompensa i el fitxer de l'objecte, o que no n'hi ha cap.
* @Parametres: in: navegant = navegant amb el viatge actual.
* @Retorn: ----.
*
************************************************/
void NAVEGANT_mostrarViatge (Navegant *navegant);

/***********************************************
*
* @Finalitat: Mostrar els recursos, el viatge i la ubicacio del navegant.
* @Parametres: in: navegant = navegant amb l'estat actual.
* @Retorn: ----.
*
************************************************/
void NAVEGANT_mostrarStatus (Navegant *navegant);

/***********************************************
*
* @Finalitat: Buscar un aliment pel nom dins de la carrega.
* @Parametres: in: navegant = navegant amb la carrega actual.
*              in: nom = nom de l'aliment que es busca.
* @Retorn: Posicio de l'aliment, o ERR_CODI si no existeix.
*
************************************************/
int NAVEGANT_buscarAliment (Navegant *navegant, char *nom);

/***********************************************
*
* @Finalitat: Obtenir la posicio d'un aliment, creant-lo a zero si es nou.
* @Parametres: in/out: navegant = navegant amb la carrega actual.
*              in: nom = nom de l'aliment que es prepara.
* @Retorn: Posicio de l'aliment, o ERR_CODI si falta memoria.
*
************************************************/
int NAVEGANT_prepararAliment (Navegant *navegant, char *nom);

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
int NAVEGANT_guardarCompra (Navegant *navegant, int index, int quantitat, char *dades, int *cost);

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
int NAVEGANT_guardarVenda (Navegant *navegant, int index, int quantitat, char *dades, int *ingressos);

/***********************************************
*
* @Finalitat: Busca una illa al mapa conegut pel seu nom.
* @Parametres:  in: navegant = navegant amb el mapa.
*               in: nom = nom de l'illa.
* @Retorn: Posicio de l'illa al mapa, o ERR_CODI si no hi es.
*
************************************************/
int NAVEGANT_buscarIllaMapa (Navegant *navegant, char *nom);

/***********************************************
*
* @Finalitat: Afegeix una illa al mapa conegut si encara no hi es.
* @Parametres:  in/out: navegant = navegant amb el mapa.
*               in: nom = nom de l'illa.
* @Retorn: Posicio de l'illa al mapa, o ERR_CODI si no hi ha memoria.
*
************************************************/
int NAVEGANT_afegirIllaMapa (Navegant *navegant, char *nom);

/***********************************************
*
* @Finalitat: Afegeix una connexio a una illa del mapa si encara no hi es.
* @Parametres:  in/out: navegant = navegant amb el mapa.
*               in: index = posicio de l'illa al mapa.
*               in: desti = nom de l'illa connectada.
* @Retorn: 0 si s'ha afegit o ja hi era, ERR_CODI si no hi ha memoria.
*
************************************************/
int NAVEGANT_afegirConnexio (Navegant *navegant, int index, char *desti);

/***********************************************
*
* @Finalitat: Afegeix al mapa una ruta bidireccional entre dues illes.
* @Parametres:  in/out: navegant = navegant amb el mapa.
*               in: origen = nom d'una de les illes.
*               in: desti = nom de l'altra illa.
* @Retorn: 0 si s'ha afegit, ERR_CODI si no hi ha memoria.
*
************************************************/
int NAVEGANT_afegirRuta (Navegant *navegant, char *origen, char *desti);

/***********************************************
*
* @Finalitat: Guarda l'adreca d'una illa si encara no es coneixia.
* @Parametres:  in/out: navegant = navegant amb les adreces.
*               in: nom = nom de l'illa.
*               in: ip = IP de l'illa.
*               in: port = port de l'illa.
* @Retorn: 0 si s'ha guardat o ja hi era, ERR_CODI si no hi ha memoria.
*
************************************************/
int NAVEGANT_afegirAdreca (Navegant *navegant, char *nom, char *ip, int port);

/***********************************************
*
* @Finalitat: Allibera una llista d'aliments i els seus noms.
* @Parametres:  in: aliments = llista d'aliments, pot ser NULL.
*               in: n_aliments = nombre d'aliments de la llista.
* @Retorn: ----.
*
************************************************/
void NAVEGANT_alliberarAliments (Food *aliments, int n_aliments);

/***********************************************
*
* @Finalitat: Escriu una linia al fitxer d'estat i l'allibera.
* @Parametres:  in: fd = fitxer d'estat obert per escriptura.
*               in: linia = text reservat amb malloc, o NULL si no s'ha pogut crear.
* @Retorn: 0 si s'ha escrit sencera, ERR_CODI si no.
*
************************************************/
int NAVEGANT_escriureLinia (int fd, char *linia);

/***********************************************
*
* @Finalitat: Escriu al fitxer d'estat la linia del viatge actual, o 0 si no n'hi ha.
* @Parametres:  in: fd = fitxer d'estat obert per escriptura.
*               in: navegant = navegant amb el viatge.
* @Retorn: 0 si s'ha escrit, ERR_CODI si no.
*
************************************************/
int NAVEGANT_escriureViatge (int fd, Navegant *navegant);

/***********************************************
*
* @Finalitat: Desa a <carpeta>/estat.db l'or, la carrega i el viatge actual amb l'objecte i si s'ha lliurat.
* @Parametres:  in: navegant = navegant amb l'estat a desar.
* @Retorn: 0 si s'ha desat, ERR_CODI si no.
*
************************************************/
int NAVEGANT_guardarEstat (Navegant *navegant);

/***********************************************
*
* @Finalitat: Buida el fitxer d'estat perque la propera execucio comenci amb els valors del fitxer de configuracio.
* @Parametres:  in: navegant = navegant amb la carpeta de l'estat.
* @Retorn: ----.
*
************************************************/
void NAVEGANT_reiniciarEstat (Navegant *navegant);

/***********************************************
*
* @Finalitat: Llegeix els aliments del fitxer d'estat i, si tots son correctes, substitueixen la carrega de la configuracio.
* @Parametres:  in/out: navegant = navegant amb la carrega.
*               in: fd = fitxer d'estat obert per lectura.
*               in: n_aliments = nombre d'aliments que s'han de llegir.
* @Retorn: 0 si s'han carregat, ERR_CODI si no.
*
************************************************/
int NAVEGANT_carregarAliments (Navegant *navegant, int fd, int n_aliments);

/***********************************************
*
* @Finalitat: Llegeix el viatge del fitxer d'estat i el recupera, amb la ruta de la imatge segellada si ja s'havia lliurat.
* @Parametres:  in/out: navegant = navegant sense viatge.
*               in: fd = fitxer d'estat obert per lectura.
* @Retorn: ----.
*
************************************************/
void NAVEGANT_carregarViatge (Navegant *navegant, int fd);

/***********************************************
*
* @Finalitat: Recupera l'estat desat a <carpeta>/estat.db. Si no existeix, esta buit o no es correcte, es mantenen els valors de la configuracio.
* @Parametres:  in/out: navegant = navegant ja inicialitzat.
* @Retorn: 0 si s'ha recuperat, ERR_CODI si no.
*
************************************************/
int NAVEGANT_carregarEstat (Navegant *navegant);

/***********************************************
*
* @Finalitat: Comprova si el navegant s'ha quedat sense sortida: menys de 50 d'or, sense menjar, sense lliurament fet i sense estar atracat al desti.
* @Parametres:  in: navegant = navegant a comprovar.
* @Retorn: SENSE_SORTIDA o AMB_SORTIDA.
*
************************************************/
int NAVEGANT_senseSortida (Navegant *navegant);

#endif