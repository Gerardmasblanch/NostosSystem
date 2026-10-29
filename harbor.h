/***********************************************
*
* @Proposit: Gestio del servidor de l'illa: connexions, port i cua d'espera.
* @Autor/s: Arnau Ricart i Gerard Mas
* @Data creacio: 29/10/2026
* @Data ultima modificacio: 29/10/2026
*
************************************************/

//Define Guards
#ifndef _HARBOR_H
#define _HARBOR_H

//Llibreries
#include <sys/select.h>

//Llibreries propies
#include "trama.h"
#include "config.h"

//Constants
#define VAIXELL_CONNECTAT 0
#define VAIXELL_ESPERANT  1
#define VAIXELL_ATRACAT   2
#define TEXT_DOCKED "DOCKED"
#define TEXT_WAIT   "WAIT"
#define TEXT_ERROR_ESTAT   "INVALID_STATE"
#define TEXT_ERROR_DADES   "INVALID_DATA"
#define TEXT_ERROR_SERVIDOR "SERVER_ERROR"
#define TEXT_OK "OK"
#define TEXT_NACK_TIPUS "INVALID_TYPE"
#define TEXT_NACK_FLAGS "INVALID_FLAGS"
#define TEXT_VAIXELL  "ship"
#define TEXT_VAIXELLS "ships"
#define MSG_DOCKED_ENVIAT ">>> DOCKED sent to %s; he may enter the port.\n"
#define MSG_APROXIMANT "\n>>> %s approaching %s.\n"
#define MSG_ATRACAT    ">>> %s entered the port.\n"
#define MSG_PORT_PLE   ">>> Port full. %s waiting offshore in the FIFO queue.\n"
#define MSG_HA_MARXAT "\n>>> %s has left.\n"
#define MSG_INICI_ILLA "Island %s initialized.\nPort capacity: %d %s.\n%d sea routes loaded.\n%d products available.\n"

//Structs
typedef struct {
    int fd;
    char *nom;
    int estat;
} Vaixell;

typedef struct {
    Vaixell *vaixells;
    int n_vaixells;
    int *cua;
    int n_cua;
    int n_atracats;
} Harbor;

//Procediments i funcions

/***********************************************
*
* @Finalitat: Deixa el port buit, sense vaixells ni cua.
* @Parametres:  out: harbor = port a inicialitzar.
* @Retorn: ----.
*
************************************************/
void inicialitzarHarbor (Harbor *harbor);

/***********************************************
*
* @Finalitat: Afegeix a la llista un vaixell nou.
* @Parametres:  in/out: harbor = port on s'afegeix el vaixell.
*               in: fd = socket retornat per l'accept.
* @Retorn: 0 si s'ha afegit, ERR_CODI si no hi ha memoria.
*
************************************************/
int afegirVaixell (Harbor *harbor, int fd);

/***********************************************
*
* @Finalitat: Treu de la cua d'espera l'element d'una posicio mantenint l'ordre.
* @Parametres:  in/out: harbor = port amb la cua.
*               in: posicio = posicio de la cua que es treu.
* @Retorn: ----.
*
************************************************/
void treureDeLaCua (Harbor *harbor, int posicio);

/***********************************************
*
* @Finalitat: Treu un vaixell de la llista, alliberant la seva placa o el seu
*             lloc a la cua, i tanca el seu socket.
* @Parametres:  in/out: harbor = port on es troba el vaixell.
*               in: index = posicio del vaixell a la llista.
* @Retorn: ----.
*
************************************************/
void treureVaixell (Harbor *harbor, int index);

/***********************************************
*
* @Finalitat: Busca un vaixell de la llista a partir del seu socket.
* @Parametres:  in: harbor = port on es busca.
*               in: fd = socket del vaixell.
* @Retorn: Posicio del vaixell a la llista, o ERR_CODI si no hi es.
*
************************************************/
int buscarVaixell (Harbor *harbor, int fd);

/***********************************************
*
* @Finalitat: Fa entrar al port els primers vaixells de la cua mentre hi hagi
*             places lliures, i els envia DOCKED.
* @Parametres:  in/out: harbor = port amb la cua.
*               in: capacitat = places totals del port.
* @Retorn: ----.
*
************************************************/
void atracarSeguent (Harbor *harbor, int capacitat);

/***********************************************
*
* @Finalitat: Afegeix un vaixell al final de la cua d'espera.
* @Parametres:  in/out: harbor = port amb la cua.
*               in: fd = socket del vaixell que espera.
* @Retorn: 0 si s'ha afegit, ERR_CODI si no hi ha memoria.
*
************************************************/
int afegirALaCua (Harbor *harbor, int fd);

/***********************************************
*
* @Finalitat: Atén l'arribada d'un vaixell: el deixa atracar si hi ha placa o
*             el posa a la cua d'espera si el port es ple.
* @Parametres:  in/out: harbor = port de l'illa.
*               in: index = posicio del vaixell a la llista.
*               in: dades = nom del vaixell rebut a la trama.
*               in: config = configuracio de l'illa.
* @Retorn: ----.
*
************************************************/
void atendreArribada (Harbor *harbor, int index, char *dades, IslandConfig *config);

/***********************************************
*
* @Finalitat: Treu un vaixell de l'illa i fa entrar el seguent de la cua si
*             s'ha alliberat una placa.
* @Parametres:  in/out: harbor = port de l'illa.
*               in: index = posicio del vaixell a la llista.
*               in: config = configuracio de l'illa.
* @Retorn: ----.
*
************************************************/
void ferSortir (Harbor *harbor, int index, IslandConfig *config);

/***********************************************
*
* @Finalitat: Atén la peticio de sortida d'un vaixell i li confirma.
* @Parametres:  in/out: harbor = port de l'illa.
*               in: index = posicio del vaixell a la llista.
*               in: config = configuracio de l'illa.
* @Retorn: ----.
*
************************************************/
void atendreSortida (Harbor *harbor, int index, IslandConfig *config);

/***********************************************
*
* @Finalitat: Omple el conjunt de descriptors que vigilara el select amb el
*             socket del servidor i els de tots els vaixells.
* @Parametres:  out: set = conjunt de descriptors a omplir.
*               in: fd_servidor = socket del servidor.
*               in: harbor = port amb la llista de vaixells.
* @Retorn: El descriptor mes gran del conjunt.
*
************************************************/
int prepararSet (fd_set *set, int fd_servidor, Harbor *harbor);

/***********************************************
*
* @Finalitat: Llegeix una trama d'un vaixell i l'atén segons el seu tipus.
*             Si la lectura falla, considera que el vaixell ha marxat.
* @Parametres:  in/out: harbor = port de l'illa.
*               in: index = posicio del vaixell a la llista.
*               in: config = configuracio de l'illa.
* @Retorn: ----.
*
************************************************/
void atendreTrama (Harbor *harbor, int index, IslandConfig *config);

/***********************************************
*
* @Finalitat: Atén tots els vaixells que el select ha marcat amb dades.
* @Parametres:  in/out: harbor = port de l'illa.
*               in: set = conjunt retornat pel select.
*               in: config = configuracio de l'illa.
* @Retorn: ----.
*
************************************************/
void atendreVaixells (Harbor *harbor, fd_set *set, IslandConfig *config);

/***********************************************
*
* @Finalitat: Tanca els sockets de tots els vaixells i allibera la memoria del port.
* @Parametres:  in/out: harbor = port a alliberar.
* @Retorn: ----.
*
************************************************/
void alliberarHarbor (Harbor *harbor);

/***********************************************
*
* @Finalitat: Mostra el missatge d'inici de l'illa.
* @Parametres:  in: config = configuracio de l'illa.
*               in: n_productes = nombre de productes del mercat.
* @Retorn: ----.
*
************************************************/
void mostrarInici (IslandConfig *config, int n_productes);

/***********************************************
*
* @Finalitat: Bucle principal del servidor: accepta connexions i atén els
*             vaixells fins que s'activa la bandera de sortida.
* @Parametres:  in: fd_servidor = socket del servidor.
*               in/out: harbor = port de l'illa.
*               in: config = configuracio de l'illa.
*               in: sortir = bandera que posa a 1 el handler de SIGINT.
* @Retorn: ----.
*
************************************************/
void executarServidor (int fd_servidor, Harbor *harbor, IslandConfig *config, int *sortir);

#endif