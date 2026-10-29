/***********************************************
*
* @Proposit: Declaracio dels tipus i funcions del servidor de viatges
*            d'Ithaca.
* @Autor/s: Arnau Ricart i Gerard Mas
* @Data creacio: 29/10/2026
* @Data ultima modificacio: 29/10/2026
*
************************************************/

//Define Guards
#ifndef _VOYAGES_H
#define _VOYAGES_H

//Llibreries
#include <sys/select.h>

//Llibreries propies
#include "trama.h"
#include "types.h"

//Constants
#define MSG_ERROR_ACCEPT "Error: no s'ha pogut acceptar la connexio.\n"
#define MSG_ERROR_DESCRIPTOR "Error: el socket supera el limit de select.\n"
#define TEXT_ITHACA_OK "OK"
#define TEXT_ITHACA_ERROR_ESTAT "INVALID_STATE"
#define TEXT_ITHACA_ERROR_DADES "INVALID_DATA"
#define TEXT_ITHACA_ERROR_SERVIDOR "SERVER_ERROR"
#define MSG_CLIENT_CONNECTAT ">>> %s connected to Ithaca.\n"

#define TEXT_ITHACA_ERROR_FLAGS "INVALID_FLAGS"
#define TEXT_ITHACA_ERROR_TIPUS "INVALID_TYPE"
#define MSG_ERROR_SELECT "Error: ha fallat el select d'Ithaca.\n"
#define MSG_ERROR_SERVIDOR "Error: no s'ha pogut obrir el servidor d'Ithaca.\n"

//Structs
typedef struct {
    int fd;
    char *nom;
} ClientIthaca;

typedef struct {
    ClientIthaca *clients;
    int n_clients;
} ServidorIthaca;

//Procediments i funcions

/***********************************************
*
* @Finalitat: Inicialitzar el servidor sense clients connectats.
* @Parametres: out: servidor = estructura del servidor a inicialitzar.
* @Retorn: ----.
*
************************************************/
void inicialitzarServidorIthaca (ServidorIthaca *servidor);

/***********************************************
*
* @Finalitat: Acceptar una connexio i afegir el client al servidor.
* @Parametres: in/out: servidor = servidor que guardara el client.
*              in: fd_server = socket que escolta connexions.
* @Retorn: 0 si s'ha afegit el client, ERR_CODI si hi ha un error.
*
************************************************/
int acceptarClientIthaca (ServidorIthaca *servidor, int fd_server);


/***********************************************
*
* @Finalitat: Preparar el conjunt de sockets que vigilara select.
* @Parametres: out: conjunt = conjunt de descriptors a preparar.
*              in: fd_server = socket que escolta connexions.
*              in: servidor = servidor amb els clients connectats.
* @Retorn: El descriptor mes gran del conjunt.
*
************************************************/
int prepararSetIthaca (fd_set *conjunt, int fd_server, ServidorIthaca *servidor);

/***********************************************
*
* @Finalitat: Tancar la connexio d'un client i treure'l de la llista.
* @Parametres: in/out: servidor = servidor amb els clients connectats.
*              in: index = posicio del client que s'ha de treure.
* @Retorn: ----.
*
************************************************/
void treureClientIthaca (ServidorIthaca *servidor, int index);

/***********************************************
*
* @Finalitat: Tancar les connexions dels clients i alliberar la seva llista.
* @Parametres: in/out: servidor = servidor que s'ha d'alliberar.
* @Retorn: ----.
*
************************************************/
void alliberarServidorIthaca (ServidorIthaca *servidor);

/***********************************************
*
* @Finalitat: Identificar un client amb el nom rebut i confirmar la connexio.
* @Parametres: in/out: servidor = servidor amb els clients connectats.
*              in: index = posicio del client que s'identifica.
*              in: dades = nom rebut al camp DATA de la trama.
* @Retorn: TRAMA_OK si s'ha enviat la resposta, TRAMA_ERROR si falla.
*
************************************************/
int atendreConnectIthaca (ServidorIthaca *servidor, int index, char *dades);

/***********************************************
*
* @Finalitat: Llegir una trama d'un client i atendre la peticio rebuda.
* @Parametres: in/out: servidor = servidor amb els clients connectats.
*              in: index = posicio del client que te activitat.
* @Retorn: ----.
*
************************************************/
void atendreTramaIthaca (ServidorIthaca *servidor, int index);

/***********************************************
*
* @Finalitat: Esperar connexions i atendre els clients amb select.
* @Parametres: in: fd_server = socket que escolta connexions.
*              in/out: servidor = servidor amb els clients connectats.
*              in: sortir = bandera activada pel handler de SIGINT.
* @Retorn: 0 si es tanca per la bandera, ERR_CODI si falla.
*
************************************************/
int executarServidorIthaca (int fd_server, ServidorIthaca *servidor, int *sortir);

#endif