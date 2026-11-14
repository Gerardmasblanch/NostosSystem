/***********************************************
*
* @Proposit: Gestio del servidor de l'illa, connexions, port i cua d'espera.
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
#include <pthread.h>

//Llibreries propies
#include "trama.h"
#include "config.h"
#include "market.h"

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
#define MSG_ERROR_PIPE   "Error: no s'ha pogut crear la pipe d'avis.\n"
#define MSG_ERROR_THREAD "Error: no s'ha pogut crear el thread del vaixell.\n"
#define MSG_ERROR_ACCEPT     "Error: no s'ha pogut acceptar la connexio.\n"
#define MSG_ERROR_DESCRIPTOR "Error: el socket supera el limit de select.\n"
#define MSG_INICI_ILLA "Island %s initialized.\nPort capacity: %d %s.\n%d sea routes loaded.\n%d products available.\n"

//Structs
typedef struct Harbor Harbor;

typedef struct {
    Harbor *harbor;
    pthread_t fil;
    int fd;
    char *nom;
    int estat;
    int acabat;
} Vaixell;

struct Harbor {
    IslandConfig *config;
    Mercat *mercat;
    Vaixell **vaixells;
    int n_vaixells;
    pthread_mutex_t mutex_vaixells;
    Vaixell **cua;
    int n_cua;
    int n_atracats;
    pthread_mutex_t mutex_port;
    int fd_avis;
};

//Procediments i funcions

/***********************************************
*
* @Finalitat: Deixa el port buit, sense vaixells ni cua, i crea els mutex de
*             la llista de vaixells i del port.
* @Parametres:  out: harbor = port a inicialitzar.
*               in: config = configuracio de l'illa.
*               in: mercat = mercat de l'illa.
*               in: fd_avis = extrem de lectura de la pipe d'avis de sortida.
* @Retorn: ----.
*
************************************************/
void HARBOR_inicialitzarHarbor (Harbor *harbor, IslandConfig *config, Mercat *mercat, int fd_avis);

/***********************************************
*
* @Finalitat: Allibera la memoria d'un vaixell. El socket ja l'ha d'haver
*             tancat el seu thread.
* @Parametres:  in/out: vaixell = vaixell a alliberar.
* @Retorn: ----.
*
************************************************/
void HARBOR_alliberarVaixell (Vaixell *vaixell);

/***********************************************
*
* @Finalitat: Accepta una connexio, afegeix el vaixell a la llista i crea
*             el seu thread.
* @Parametres:  in/out: harbor = port on s'afegeix el vaixell.
*               in: fd_servidor = socket que escolta connexions.
* @Retorn: 0 si s'ha afegit, ERR_CODI si hi ha un error.
*
************************************************/
int HARBOR_acceptarVaixell (Harbor *harbor, int fd_servidor);

/***********************************************
*
* @Finalitat: Fa el join i allibera els vaixells el thread dels quals ja ha
*             acabat, i els treu de la llista.
* @Parametres:  in/out: harbor = port amb la llista de vaixells.
* @Retorn: ----.
*
************************************************/
void HARBOR_netejarVaixellsAcabats (Harbor *harbor);

/***********************************************
*
* @Finalitat: Espera (join) que acabin els threads de tots els vaixells que
*             queden en tancar l'illa. No agafa el mutex de vaixells perque
*             els threads el necessiten per acabar.
* @Parametres:  in/out: harbor = port amb la llista de vaixells.
* @Retorn: ----.
*
************************************************/
void HARBOR_esperarVaixells (Harbor *harbor);

/***********************************************
*
* @Finalitat: Funcio del thread de cada vaixell: atén les seves trames fins
*             que marxa, cau la connexio o s'avisa la sortida de l'illa.
* @Parametres:  in/out: arg = punter al Vaixell que atén el thread.
* @Retorn: NULL.
*
************************************************/
void *HARBOR_atendreVaixell (void *arg);

/***********************************************
*
* @Finalitat: Treu de la cua d'espera l'element d'una posicio mantenint l'ordre.
* @Parametres:  in/out: harbor = port amb la cua.
*               in: posicio = posicio de la cua que es treu.
* @Retorn: ----.
*
************************************************/
void HARBOR_treureDeLaCua (Harbor *harbor, int posicio);

/***********************************************
*
* @Finalitat: Treu un vaixell de la llista, alliberant la seva plaça o el seu
*             lloc a la cua, i tanca el seu socket.
* @Parametres:  in/out: harbor = port on es troba el vaixell.
*               in: index = posicio del vaixell a la llista.
* @Retorn: ----.
*
************************************************/
void HARBOR_treureVaixell (Harbor *harbor, int index);

/***********************************************
*
* @Finalitat: Busca un vaixell de la llista a partir del seu socket.
* @Parametres:  in: harbor = port on es busca.
*               in: fd = socket del vaixell.
* @Retorn: Posicio del vaixell a la llista, o ERR_CODI si no hi es.
*
************************************************/
int HARBOR_buscarVaixell (Harbor *harbor, int fd);

/***********************************************
*
* @Finalitat: Fa entrar al port els primers vaixells de la cua mentre hi hagi
*             places lliures, i els envia DOCKED.
* @Parametres:  in/out: harbor = port amb la cua.
*               in: capacitat = places totals del port.
* @Retorn: ----.
*
************************************************/
void HARBOR_atracarSeguent (Harbor *harbor, int capacitat);

/***********************************************
*
* @Finalitat: Afegeix un vaixell al final de la cua d'espera.
* @Parametres:  in/out: harbor = port amb la cua.
*               in: fd = socket del vaixell que espera.
* @Retorn: 0 si s'ha afegit, ERR_CODI si no hi ha memoria.
*
************************************************/
int HARBOR_afegirALaCua (Harbor *harbor, int fd);

/***********************************************
*
* @Finalitat: Atén l'arribada d'un vaixell, el deixa TRAVESSIA_atracar si hi ha plaça o
*             el posa a la cua d'espera si el port es ple.
* @Parametres:  in/out: harbor = port de l'illa.
*               in: index = posicio del vaixell a la llista.
*               in: dades = nom del vaixell rebut a la trama.
*               in: config = configuracio de l'illa.
* @Retorn: ----.
*
************************************************/
void HARBOR_atendreArribada (Harbor *harbor, int index, char *dades, IslandConfig *config);

/***********************************************
*
* @Finalitat: Treu un vaixell de l'illa i fa entrar el seguent de la cua si
*             s'ha alliberat una plaça.
* @Parametres:  in/out: harbor = port de l'illa.
*               in: index = posicio del vaixell a la llista.
*               in: config = configuracio de l'illa.
* @Retorn: ----.
*
************************************************/
void HARBOR_ferSortir (Harbor *harbor, int index, IslandConfig *config);

/***********************************************
*
* @Finalitat: Atén la peticio de sortida d'un vaixell i li confirma.
* @Parametres:  in/out: harbor = port de l'illa.
*               in: index = posicio del vaixell a la llista.
*               in: config = configuracio de l'illa.
* @Retorn: ----.
*
************************************************/
void HARBOR_atendreSortida (Harbor *harbor, int index, IslandConfig *config);

/***********************************************
*
* @Finalitat: Atén una peticio del mercat (LIST MARKET, BUY, SELL o BUY MAP)
*             si el vaixell esta atracat.
* @Parametres:  in: harbor = port de l'illa.
*               in: index = posicio del vaixell a la llista.
*               in: tipus = tipus de la trama rebuda.
*               in: dades = text de la peticio.
*               in: config = configuracio de l'illa.
*               in/out: mercat = mercat de l'illa.
* @Retorn: ----.
*
************************************************/
void HARBOR_atendreMercat (Harbor *harbor, int index, int tipus, char *dades, IslandConfig *config, Mercat *mercat);


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
void HARBOR_atendreTrama (Harbor *harbor, int index, IslandConfig *config, Mercat *mercat);

/***********************************************
*
* @Finalitat: Allibera els vaixells, la cua i els mutex del port. Nomes es
*             pot cridar quan ja s'ha fet el join de tots els threads.
* @Parametres:  in/out: harbor = port a alliberar.
* @Retorn: ----.
*
************************************************/
void HARBOR_alliberarHarbor (Harbor *harbor);

/***********************************************
*
* @Finalitat: Mostra el missatge d'inici de l'illa.
* @Parametres:  in: config = configuracio de l'illa.
*               in: n_productes = nombre de productes del mercat.
* @Retorn: ----.
*
************************************************/
void HARBOR_mostrarInici (IslandConfig *config, int n_productes);

/***********************************************
*
* @Finalitat: Bucle del fil principal: accepta vaixells i crea el thread de
*             cadascun fins que la pipe d'avis indica la sortida. En acabar,
*             espera tots els threads.
* @Parametres:  in: fd_servidor = socket del servidor.
*               in/out: harbor = port de l'illa.
* @Retorn: ----.
*
************************************************/
void HARBOR_executarServidor (int fd_servidor, Harbor *harbor);

#endif