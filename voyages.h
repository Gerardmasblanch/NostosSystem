/***********************************************
*
* @Proposit: Declaracio dels tipus i funcions del servidor de viatges
*            d'Ithaca.
* @Autor/s: Arnau Ricart i Gerard Mas
* @Data creacio: 29/10/2026
* @Data ultima modificacio: 29/10/2026
*
************************************************/

#ifndef _VOYAGES_H
#define _VOYAGES_H

#include <sys/select.h>

#include "trama.h"
#include "types.h"

#define MSG_ERROR_SERVIDOR "Error: no s'ha pogut obrir el servidor d'Ithaca.\n" 

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
#define MSG_CLIENT_MARXA ">>> %s has left Ithaca. Fair winds!\n"

#define VIATGE_DISPONIBLE 0
#define VIATGE_ASSIGNAT 1
#define VIATGE_FRACASSAT 2

#define FORMAT_VIATGE_DISPONIBLE "%d&%d&%d&%s&%s&%d"
#define TEXT_ITHACA_SENSE_VIATGES "0"

#define MSG_CONSULTA_VIATGES ">>> %s requested available voyages.\n"

#define TEXT_ITHACA_ERROR_NOM "NAME_EXISTS"

#define TEXT_ITHACA_ERROR_DISPONIBILITAT "VOYAGE_NOT_AVAILABLE"
#define FORMAT_VIATGE_ACCEPTAT "OK&%d&%s&%s&%d"

#define MSG_VIATGE_ASSIGNAT ">>> Voyage %d assigned to %s.\n"

typedef struct {
    int fd;
    char *nom;
    int viatge_acceptat;
    int *viatges_consultats;
    int n_viatges_consultats;
} ClientIthaca;

typedef struct {
    Voyage *viatge;
    int estat;
    char *nom_odysseus;
} ViatgeIthaca;

typedef struct {
    ClientIthaca *clients;
    int n_clients;
    ViatgeIthaca *viatges;
    int n_viatges;
} ServidorIthaca;

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
* @Finalitat: Preparar l'estat dels viatges en un servidor inicialitzat.
* @Parametres: in/out: servidor = servidor amb la llista de viatges buida.
*              in: viatges = array de viatges carregats del fitxer.
*              in: n_viatges = nombre de viatges carregats.
* @Retorn: 0 si s'han preparat, ERR_CODI si falta memoria.
*
************************************************/
int prepararViatgesIthaca (ServidorIthaca *servidor, Voyage *viatges, int n_viatges);

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

/***********************************************
*
* @Finalitat: Comptar els viatges que estan disponibles.
* @Parametres: in: servidor = servidor amb els estats dels viatges.
* @Retorn: Nombre de viatges disponibles.
*
************************************************/
int comptarViatgesDisponibles (ServidorIthaca *servidor);

/***********************************************
*
* @Finalitat: Enviar una trama per cada viatge disponible.
* @Parametres: in: servidor = servidor amb els viatges i els seus estats.
*              in: fd = socket del client que rebra la llista.
* @Retorn: TRAMA_OK si s'han enviat les respostes, TRAMA_ERROR si falla.
*
************************************************/
int enviarViatgesDisponibles (ServidorIthaca *servidor, int fd);

/***********************************************
*
* @Finalitat: Buscar el viatge assignat a un navegant.
* @Parametres: in: servidor = servidor amb els estats dels viatges.
*              in: nom = nom del navegant que es busca.
* @Retorn: Posicio del viatge assignat, o ERR_CODI si no en te cap.
*
************************************************/
int buscarViatgeAssignat (ServidorIthaca *servidor, char *nom);

/***********************************************
*
* @Finalitat: Guardar els IDs dels viatges disponibles per a un client.
* @Parametres: in: servidor = servidor amb els viatges disponibles.
*              in: index = posicio del client que consulta els viatges.
* @Retorn: 0 si s'han guardat, ERR_CODI si falta memoria.
*
************************************************/
int guardarViatgesConsultats (ServidorIthaca *servidor, int index);

/***********************************************
*
* @Finalitat: Validar i atendre una consulta dels viatges disponibles.
* @Parametres: in/out: servidor = servidor amb els clients i viatges.
*              in: index = posicio del client que fa la consulta.
*              in: dades = camp DATA de la peticio, que ha de ser buit.
* @Retorn: TRAMA_OK si s'ha enviat la resposta, TRAMA_ERROR si falla.
*
************************************************/
int atendreListVoyages (ServidorIthaca *servidor, int index, char *dades);

/***********************************************
*
* @Finalitat: Comprovar si un nom ja pertany a un client connectat.
* @Parametres: in: servidor = servidor amb els clients connectats.
*              in: nom = nom que es vol comprovar.
* @Retorn: 1 si el nom ja existeix, 0 si no existeix.
*
************************************************/
int existeixNomIthaca (ServidorIthaca *servidor, char *nom);

/***********************************************
*
* @Finalitat: Comprovar si un viatge apareix a l'ultima consulta del client.
* @Parametres: in: client = client amb els IDs dels viatges consultats.
*              in: id_viatge = identificador del viatge que es comprova.
* @Retorn: 1 si el viatge apareix a la consulta, 0 si no hi apareix.
*
************************************************/
int haConsultatViatge (ClientIthaca *client, int id_viatge);

/***********************************************
*
* @Finalitat: Buscar un viatge a partir del seu identificador.
* @Parametres: in: servidor = servidor amb la llista de viatges.
*              in: id_viatge = identificador del viatge que es busca.
* @Retorn: Posicio del viatge, o ERR_CODI si no existeix.
*
************************************************/
int buscarViatgePerId (ServidorIthaca *servidor, int id_viatge);

/***********************************************
*
* @Finalitat: Validar una peticio ACCEPT i assignar el viatge al client.
* @Parametres: in/out: servidor = servidor amb els clients i viatges.
*              in: index = posicio del client que accepta.
*              in: dades = identificador del viatge en format de text.
* @Retorn: TRAMA_OK si s'ha enviat la resposta, TRAMA_ERROR si falla.
*
************************************************/
int atendreAcceptIthaca (ServidorIthaca *servidor, int index, char *dades);

#endif