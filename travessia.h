/***********************************************
*
* @Proposit: Travessia de l'Odysseus: bucle principal amb select i execucio
*            de les comandes del navegant.
* @Autor/s: Arnau Ricart i Gerard Mas
* @Data creacio: 29/10/2026
* @Data ultima modificacio: 29/10/2026
*
************************************************/

//Define Guards
#ifndef _TRAVESSIA_H
#define _TRAVESSIA_H

//Llibreries
#include <sys/select.h>

//Llibreries propies
#include "commands.h"
#include "navegant.h"

//Constants
#define FINAL_NO 0
#define FINAL_SI 1
#define RESPOSTA_OK      0
#define RESPOSTA_ERROR   1
#define RESPOSTA_PERDUDA 2

#define TEXT_PROMPT "$ "
#define MSG_DESCONEGUDA "Unknown command\n"
#define MSG_COMANDA_OK  "Command OK\n"
#define MSG_CONNEXIO_PERDUDA "\nLost connection with %s.\n"
#define MSG_CONNECTAT      "Connected to %s.\n"
#define MSG_ERROR_CONNEXIO "Error: could not reach %s.\n"
#define MSG_ERROR_RESPOSTA "Error: %s.\n"
#define MSG_JA_CONNECTAT   "You are already connected to Ithaca.\n"
#define MSG_NOMES_A_ITHACA "You can only connect to Ithaca from Ithaca.\n"
#define TEXT_SENSE_VIATGES "0"
#define MSG_CAPCALERA_VIATGES "\n--- AVAILABLE VOYAGES ---\n\n"
#define FORMAT_VIATGE "%s. %-20s -> %-12s [%s]\n"
#define MSG_SENSE_VIATGES "No voyages available.\n"
#define MSG_NO_CONNECTAT_ITHACA "You must be connected to Ithaca.\n"
#define MSG_RESPOSTA_INCORRECTA "Error: unexpected response from the server.\n"

//Procediments i funcions

/***********************************************
*
* @Finalitat: Omple el conjunt de descriptors amb el teclat i, si n'hi ha,
*             la connexio actual.
* @Parametres:  out: set = conjunt de descriptors a omplir.
*               in: navegant = navegant amb la connexio actual.
* @Retorn: El descriptor mes gran del conjunt.
*
************************************************/
int prepararSetOdysseus (fd_set *set, Navegant *navegant);

/***********************************************
*
* @Finalitat: Tanca la connexio actual perque s'ha perdut i actualitza
*             l'estat del navegant segons on es trobi.
* @Parametres:  in/out: navegant = navegant amb la connexio actual.
* @Retorn: ----.
*
************************************************/
void perdreConnexio (Navegant *navegant);

/***********************************************
*
* @Finalitat: Espera la resposta del servidor a una peticio i comprova que
*             sigui del tipus esperat i correcta.
* @Parametres:  in/out: navegant = navegant amb la connexio actual.
*               in: tipus_esperat = tipus de la peticio enviada.
*               out: dades = dades de la resposta, reservades amb malloc.
* @Retorn: RESPOSTA_OK, RESPOSTA_ERROR o RESPOSTA_PERDUDA.
*
************************************************/
int rebreResposta (Navegant *navegant, int tipus_esperat, char **dades);


/***********************************************
*
* @Finalitat: Atén una trama no esperada del servidor. Si la connexio s'ha
*             perdut, la tanca i actualitza l'estat del navegant.
* @Parametres:  in/out: navegant = navegant amb la connexio actual.
* @Retorn: ----.
*
************************************************/
void atendreServidor (Navegant *navegant);


/***********************************************
*
* @Finalitat: Connecta el navegant amb Ithaca i s'hi identifica amb el seu nom.
* @Parametres:  in/out: navegant = estat del navegant.
* @Retorn: ----.
*
************************************************/
void ferConnectIthaca (Navegant *navegant);


/***********************************************
*
* @Finalitat: Interpreta una trama de LIST VOYAGES i mostra el viatge.
* @Parametres:  in: dades = <Index>&<Total>&<VoyageID>&<Object>&<Destination>&<Reward>.
*               out: total = nombre total de viatges que enviara Ithaca.
* @Retorn: TRAMA_OK si el format es correcte, TRAMA_ERROR si no.
*
************************************************/
int mostrarViatge (char *dades, int *total);

/***********************************************
*
* @Finalitat: Demana a Ithaca els viatges disponibles i els mostra.
* @Parametres:  in/out: navegant = estat del navegant.
* @Retorn: ----.
*
************************************************/
void ferListVoyages (Navegant *navegant);

/***********************************************
*
* @Finalitat: Interpreta una linia del teclat i executa la comanda.
* @Parametres:  in/out: navegant = estat del navegant.
*               in: linea = linia escrita per l'usuari.
* @Retorn: FINAL_SI si l'Odysseus ha d'acabar, FINAL_NO si no.
*
************************************************/


int executarComanda (Navegant *navegant, char *linea);

/***********************************************
*
* @Finalitat: Llegeix una linia del teclat i l'executa.
* @Parametres:  in/out: navegant = estat del navegant.
* @Retorn: FINAL_SI si s'ha tancat l'entrada o l'Odysseus ha d'acabar,
*          FINAL_NO si no.
*
************************************************/
int atendreTeclat (Navegant *navegant);

/***********************************************
*
* @Finalitat: Bucle principal de l'Odysseus: atén el teclat i la connexio
*             fins que s'activa la bandera de sortida o s'ha d'acabar.
* @Parametres:  in/out: navegant = estat del navegant.
*               in: sortir = bandera que posa a 1 el handler de SIGINT.
* @Retorn: ----.
*
************************************************/
void executarOdysseus (Navegant *navegant, int *sortir);



#endif