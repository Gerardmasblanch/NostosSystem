/***********************************************
*
* @Proposit: Declaracio dels tipus i funcions del servidor de viatges
*            d'Ithaca. El fil principal accepta connexions i cada client
*            s'atén en un thread propi; la taula de viatges i la llista de
*            clients es protegeixen amb un mutex cadascuna.
* @Autor/s: Arnau Ricart i Gerard Mas
* @Data creacio: 29/10/2026
* @Data ultima modificacio: 17/11/2026
*
************************************************/

#ifndef _VOYAGES_H
#define _VOYAGES_H

#include <sys/select.h>
#include <pthread.h>

#include "trama.h"
#include "types.h"
#include "fitxer.h"
#include "sphragis.h"

#define MSG_ERROR_SERVIDOR "Error: no s'ha pogut obrir el servidor d'Ithaca.\n"

#define MSG_ERROR_PIPE "Error: no s'ha pogut crear la pipe d'avis.\n"
#define MSG_ERROR_THREAD "Error: no s'ha pogut crear el thread del client.\n"
#define MSG_ERROR_ACCEPT "Error: no s'ha pogut acceptar la connexio.\n"
#define MSG_ERROR_DESCRIPTOR "Error: el socket supera el limit de select.\n"
#define TEXT_ITHACA_OK "OK"
#define TEXT_ITHACA_ERROR_ESTAT "INVALID_STATE"
#define TEXT_ITHACA_ERROR_DADES "INVALID_DATA"
#define TEXT_ITHACA_ERROR_SERVIDOR "SERVER_ERROR"
#define MSG_CLIENT_CONNECTAT ">>> %s connected to Ithaca.\n"
#define MSG_CLIENT_TORNAT ">>> %s returned to Ithaca.\n"

#define TEXT_ITHACA_ERROR_FLAGS "INVALID_FLAGS"
#define TEXT_ITHACA_ERROR_TIPUS "INVALID_TYPE"
#define MSG_CLIENT_MARXA ">>> %s has left Ithaca. Fair winds!\n"

#define CLIENT_CONTINUA 1
#define CLIENT_ACABA    0

#define VIATGE_DISPONIBLE 0
#define VIATGE_ASSIGNAT 1
#define VIATGE_FRACASSAT 2
#define VIATGE_COMPLETAT 3

#define PREFIX_CLAIM "claim_"
#define FORMAT_CLAIM_OK "OK&%d"
#define TEXT_ITHACA_ERROR_GLIF "INVALID_GLYPH"
#define TEXT_ITHACA_ERROR_OBJECTE "INVALID_OBJECT"
#define TEXT_ITHACA_ERROR_TRANSFERENCIA "TRANSFER_ERROR"
#define MSG_CLAIM_PETICIO    ">>> %s claims voyage %s.\n"
#define MSG_CLAIM_REBENT     "\n>>> Receiving stamped %s...\n"
#define MSG_CLAIM_REBUT      ">>> Object received correctly.\n"
#define MSG_CLAIM_GLIF       "\n>>> Glyph of %s verified.\n"
#define MSG_CLAIM_COMPLETAT  ">>> Voyage %d completed by %s.\n"
#define MSG_CLAIM_RECOMPENSA ">>> Reward granted: %d gold.\n"
#define MSG_CLAIM_REBUTJAT   ">>> Claim of voyage %s by %s rejected.\n"

#define FORMAT_VIATGE_DISPONIBLE "%d&%d&%d&%s&%s&%d"
#define TEXT_ITHACA_SENSE_VIATGES "0"

#define MSG_CONSULTA_VIATGES ">>> %s requested available voyages.\n"

#define TEXT_ITHACA_ERROR_NOM "NAME_EXISTS"

#define TEXT_ITHACA_ERROR_DISPONIBILITAT "VOYAGE_NOT_AVAILABLE"
#define FORMAT_VIATGE_ACCEPTAT "OK&%d&%s&%s&%d"

#define MSG_VIATGE_ASSIGNAT ">>> Voyage %d assigned to %s.\n"
#define MSG_VIATGE_FRACASSAT ">>> Voyage %d of %s marked as failed.\n"
#define MSG_ENVIANT_OBJECTE ">>> Sending %s to %s...\n"
#define MSG_OBJECTE_ENVIAT  "\n>>> Object transferred successfully.\n"
#define MSG_OBJECTE_FALLAT  ">>> Object transfer to %s failed. Voyage %d marked as failed.\n"

typedef struct ServidorIthaca ServidorIthaca;

typedef struct {
    ServidorIthaca *servidor;
    pthread_t fil;
    int fd;
    char *nom;
    int viatge_acceptat;
    int *viatges_consultats;
    int n_viatges_consultats;
    int acabat;
} ClientIthaca;

typedef struct {
    Voyage *viatge;
    int estat;
    char *nom_odysseus;
} ViatgeIthaca;

struct ServidorIthaca {
    ViatgeIthaca *viatges;
    int n_viatges;
    pthread_mutex_t mutex_viatges;
    ClientIthaca **clients;
    int n_clients;
    pthread_mutex_t mutex_clients;
    int fd_avis;
    char *carpeta;
};

/***********************************************
*
* @Finalitat: Inicialitzar el servidor sense clients ni viatges i crear els
*             mutex de la taula de viatges i de la llista de clients.
* @Parametres: out: servidor = estructura del servidor a inicialitzar.
*              in: carpeta = carpeta d'Ithaca on es desen les imatges del CLAIM.
*              in: fd_avis = extrem de lectura de la pipe d'avis de sortida.
* @Retorn: ----.
*
************************************************/
void VOYAGES_inicialitzarServidorIthaca (ServidorIthaca *servidor, char *carpeta, int fd_avis);

/***********************************************
*
* @Finalitat: Alliberar la memoria d'un client. El socket ja l'ha d'haver
*             tancat el seu thread.
* @Parametres: in/out: client = client a alliberar.
* @Retorn: ----.
*
************************************************/
void VOYAGES_alliberarClient (ClientIthaca *client);

/***********************************************
*
* @Finalitat: Preparar l'estat dels viatges en un servidor inicialitzat.
*             Es crida abans de crear cap thread.
* @Parametres: in/out: servidor = servidor amb la llista de viatges buida.
*              in: viatges = array de viatges carregats del fitxer.
*              in: n_viatges = nombre de viatges carregats.
* @Retorn: 0 si s'han preparat, ERR_CODI si falta memoria.
*
************************************************/
int VOYAGES_prepararViatgesIthaca (ServidorIthaca *servidor, Voyage *viatges, int n_viatges);

/***********************************************
*
* @Finalitat: Buscar el viatge assignat a un Odysseus. S'ha de cridar amb
*             mutex_viatges agafat.
* @Parametres: in: servidor = servidor amb els viatges.
*              in: nom = nom de l'Odysseus.
* @Retorn: Posicio del viatge o ERR_CODI si no en te cap d'assignat.
*
************************************************/
int VOYAGES_buscarViatgeAssignat (ServidorIthaca *servidor, char *nom);

/***********************************************
*
* @Finalitat: Buscar un viatge pel seu identificador. L'identificador no
*             canvia mai, pero es crida amb mutex_viatges agafat perque
*             forma part de la regio de l'ACCEPT.
* @Parametres: in: servidor = servidor amb els viatges.
*              in: id_viatge = identificador a buscar.
* @Retorn: Posicio del viatge o ERR_CODI si no existeix.
*
************************************************/
int VOYAGES_buscarViatgePerId (ServidorIthaca *servidor, int id_viatge);

/***********************************************
*
* @Finalitat: Comptar els viatges disponibles. S'ha de cridar amb
*             mutex_viatges agafat.
* @Parametres: in: servidor = servidor amb els viatges.
* @Retorn: Nombre de viatges disponibles.
*
************************************************/
int VOYAGES_comptarViatgesDisponibles (ServidorIthaca *servidor);

/***********************************************
*
* @Finalitat: Desar els identificadors dels viatges disponibles que ha vist
*             el client al seu ultim LIST VOYAGES. S'ha de cridar amb
*             mutex_viatges agafat.
* @Parametres: in: servidor = servidor amb els viatges.
*              in/out: client = client que ha fet la consulta.
* @Retorn: 0 si s'han desat, ERR_CODI si falta memoria.
*
************************************************/
int VOYAGES_guardarViatgesConsultats (ServidorIthaca *servidor, ClientIthaca *client);

/***********************************************
*
* @Finalitat: Comprovar si un viatge era a l'ultim LIST VOYAGES del client.
*             Nomes ho fa servir el thread del mateix client.
* @Parametres: in: client = client que vol acceptar el viatge.
*              in: id_viatge = identificador del viatge.
* @Retorn: 1 si l'havia consultat, 0 si no.
*
************************************************/
int VOYAGES_haConsultatViatge (ClientIthaca *client, int id_viatge);

/***********************************************
*
* @Finalitat: Copiar el text de cada viatge disponible amb el format del
*             protocol. S'ha de cridar amb mutex_viatges agafat; els textos
*             s'envien despres de deixar-lo.
* @Parametres: in: servidor = servidor amb els viatges.
*              out: textos = array de textos creat (NULL si no n'hi ha).
* @Retorn: Nombre de textos, o ERR_CODI si falta memoria.
*
************************************************/
int VOYAGES_copiarViatgesDisponibles (ServidorIthaca *servidor, char ***textos);

/***********************************************
*
* @Finalitat: Enviar els textos dels viatges disponibles, o "0" si no n'hi
*             ha cap. Es crida sense cap mutex agafat.
* @Parametres: in: fd = socket del client.
*              in: textos = textos a enviar.
*              in: n_textos = nombre de textos.
* @Retorn: TRAMA_OK o TRAMA_ERROR.
*
************************************************/
int VOYAGES_enviarViatges (int fd, char **textos, int n_textos);

/***********************************************
*
* @Finalitat: Alliberar un array de textos.
* @Parametres: in/out: textos = array a alliberar (pot ser NULL).
*              in: n_textos = nombre de textos de l'array.
* @Retorn: ----.
*
************************************************/
void VOYAGES_alliberarTextos (char **textos, int n_textos);

/***********************************************
*
* @Finalitat: Atendre LIST VOYAGES: marcar com a fracassat el viatge que
*             encara tingues assignat l'Odysseus, desar els consultats i
*             enviar els disponibles.
* @Parametres: in/out: client = client que fa la peticio.
*              in: dades = camp DATA de la trama rebuda.
* @Retorn: TRAMA_OK o TRAMA_ERROR.
*
************************************************/
int VOYAGES_atendreListVoyages (ClientIthaca *client, char *dades);

/***********************************************
*
* @Finalitat: Atendre ACCEPT: comprovar que el viatge esta disponible i
*             assignar-lo a l'Odysseus dins d'una mateixa regio critica.
* @Parametres: in/out: client = client que fa la peticio.
*              in: dades = identificador del viatge.
* @Retorn: TRAMA_OK o TRAMA_ERROR.
*
************************************************/
int VOYAGES_atendreAcceptIthaca (ClientIthaca *client, char *dades);

/***********************************************
*
* @Finalitat: Envia la imatge de l'objecte (transferencia OBJECT) just
*             despres d'un ACCEPT correcte. Si falla, el viatge passa a
*             fracassat i el client pot acceptar-ne un altre.
* @Parametres:  in/out: client = client que ha acceptat el viatge.
*               in: index_viatge = posicio del viatge acceptat.
* @Retorn: TRAMA_OK si s'ha de seguir atenent el client; TRAMA_ERROR si
*          la transferencia ha fallat per comunicacio i s'ha de tancar.
*
************************************************/
int VOYAGES_enviarObjecte (ClientIthaca *client, int index_viatge);

/***********************************************
*
* @Finalitat: Informa d'un CLAIM rebutjat i respon 0x04 amb l'error.
* @Parametres:  in: client = client que ha fet el CLAIM.
*               in: id_text = identificador del viatge rebut.
*               in: motiu = text d'error de la resposta.
* @Retorn: TRAMA_OK o TRAMA_ERROR.
*
************************************************/
int VOYAGES_rebutjarClaim (ClientIthaca *client, char *id_text, char *motiu);

/***********************************************
*
* @Finalitat: Comprova que la imatge rebuda en un CLAIM es l'objecte del viatge segellat
*             per la illa de destinacio, sense crear cap fitxer nou: calcula l'MD5 del fitxer
*             rebut, hi torna a segellar l'original a sobre i compara els dos MD5. Si coincideixen,
*             el fitxer queda identic al rebut.
* @Parametres:  in: viatge = viatge que es reclama.
*               in: path_rebut = ruta de la imatge rebuda, que es sobreescriu.
* @Retorn: 1 si es l'objecte del viatge, 0 si no.
*
************************************************/
int VOYAGES_esObjecteDelViatge (Voyage *viatge, char *path_rebut);

/***********************************************
*
* @Finalitat: Atén el CLAIM: rep la imatge segellada, comprova que es
*             l'objecte del viatge, que te el glif de l'illa de desti i que
*             el viatge esta assignat a l'Odysseus, el marca com a
*             completat i respon OK&Reward. Comprovar l'assignacio i
*             marcar-lo completat es fa en un sol lock: mai es paga dos cops.
* @Parametres:  in/out: client = client que reclama la recompensa.
*               in: dades = identificador del viatge.
* @Retorn: TRAMA_OK si s'ha de seguir atenent el client; TRAMA_ERROR si
*          s'ha de tancar la connexio.
*
************************************************/
int VOYAGES_atendreClaim (ClientIthaca *client, char *dades);

/***********************************************
*
* @Finalitat: Acceptar una connexio, afegir el client a la llista i crear
*             el seu thread.
* @Parametres: in/out: servidor = servidor que guardara el client.
*              in: fd_server = socket que escolta connexions.
* @Retorn: 0 si s'ha afegit el client, ERR_CODI si hi ha un error.
*
************************************************/
int VOYAGES_acceptarClientIthaca (ServidorIthaca *servidor, int fd_server);

/***********************************************
*
* @Finalitat: Fer el join i alliberar els clients el thread dels quals ja
*             ha acabat, i treure'ls de la llista.
* @Parametres: in/out: servidor = servidor amb la llista de clients.
* @Retorn: ----.
*
************************************************/
void VOYAGES_netejarClientsAcabats (ServidorIthaca *servidor);

/***********************************************
*
* @Finalitat: Esperar (join) que acabin els threads de tots els clients
*             que queden connectats en tancar Ithaca. No agafa el mutex de
*             clients perque els threads el necessiten per acabar.
* @Parametres: in/out: servidor = servidor amb la llista de clients.
* @Retorn: ----.
*
************************************************/
void VOYAGES_esperarClients (ServidorIthaca *servidor);

/***********************************************
*
* @Finalitat: Alliberar els clients, els viatges i els mutex del servidor.
*             Nomes es pot cridar quan ja s'ha fet el join de tots els
*             threads.
* @Parametres: in/out: servidor = servidor que s'ha d'alliberar.
* @Retorn: ----.
*
************************************************/
void VOYAGES_alliberarServidorIthaca (ServidorIthaca *servidor);

/***********************************************
*
* @Finalitat: Comprovar si un nom ja el fa servir un client connectat que
*             no ha acabat. S'ha de cridar amb mutex_clients agafat.
* @Parametres: in: servidor = servidor amb els clients connectats.
*              in: nom = nom que es vol comprovar.
* @Retorn: 1 si el nom ja existeix, 0 si no.
*
************************************************/
int VOYAGES_existeixNomIthaca (ServidorIthaca *servidor, char *nom);

/***********************************************
*
* @Finalitat: Atendre CONNECT: comprovar que el nom no existeix i desar-lo
*             dins d'una mateixa regio critica, i confirmar la connexio.
* @Parametres: in/out: client = client que s'identifica.
*              in: dades = nom de l'Odysseus.
* @Retorn: TRAMA_OK o TRAMA_ERROR.
*
************************************************/
int VOYAGES_atendreConnectIthaca (ClientIthaca *client, char *dades);

/***********************************************
*
* @Finalitat: Rebre una trama del client i respondre-la.
* @Parametres: in/out: client = client que ha enviat la trama.
* @Retorn: CLIENT_CONTINUA si s'ha de seguir atenent el client,
*          CLIENT_ACABA si ha marxat o s'ha perdut la connexio.
*
************************************************/
int VOYAGES_atendreTramaIthaca (ClientIthaca *client);

/***********************************************
*
* @Finalitat: Funcio del thread de cada client: atén les seves trames fins
*             que marxa, cau la connexio o s'avisa la sortida d'Ithaca.
* @Parametres: in/out: arg = punter al ClientIthaca que atén el thread.
* @Retorn: NULL.
*
************************************************/
void *VOYAGES_atendreClient (void *arg);

/***********************************************
*
* @Finalitat: Bucle del fil principal: espera connexions i crea un thread
*             per a cada client fins que la pipe d'avis indica la sortida.
*             En acabar, espera tots els threads.
* @Parametres: in: fd_server = socket que escolta connexions.
*              in/out: servidor = servidor amb els clients connectats.
* @Retorn: 0 si es tanca per l'avis de sortida, ERR_CODI si falla.
*
************************************************/
int VOYAGES_executarServidorIthaca (int fd_server, ServidorIthaca *servidor);

#endif
