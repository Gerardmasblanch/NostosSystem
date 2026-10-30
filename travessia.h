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
#include <signal.h>

//Llibreries propies
#include "commands.h"
#include "navegant.h"

//Constants
#define FINAL_NO 0
#define FINAL_SI 1
#define RESPOSTA_OK      0
#define RESPOSTA_ERROR   1
#define RESPOSTA_PERDUDA 2
#define RACIO_KG     7
#define SEGONS_RACIO 5

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
#define MSG_VIATGE_ACCEPTAT "Voyage %d accepted.\nDestination: %s.\n"
#define MSG_JA_TE_VIATGE    "You already have a voyage assigned.\n"
#define TEXT_ATRACAT "DOCKED"
#define TEXT_ESPERA  "WAIT"
#define MSG_NO_ES_POT_NAVEGAR "We can't sail to %s from here...\n"
#define MSG_MARXANT           "Leaving %s...\n"
#define MSG_NAVEGANT          "Sailing to %s...\n"
#define MSG_ACCES             "%s has granted access to the port.\n"
#define MSG_ACCES_ESPERA      "\n>>> %s has granted access to the port.\n"
#define MSG_PORT_COMPLET      "The harbor is full. We drop anchor and wait.\n"
#define MSG_RACIO             "\n>>> The crew consumes another ration.\nProvisions remaining: %d kg\n"
#define MSG_SENSE_PROVISIONS  "Provisions remaining: 0 kg\nThe crew has run out of provisions.\nOdysseus %s has been lost at sea.\n"
#define MSG_DESTI             "We have reached %s, the destination of our voyage.\n"
#define MOSTRAR_NO 0
#define MOSTRAR_SI 1

#define TEXT_MAPA        "MAP"
#define TEXT_MAPA_MERCAT "Map"
#define MSG_CAPCALERA_MERCAT   "\n--- %s MARKET ---\n\n"
#define FORMAT_PRODUCTE_MERCAT "%-15s %4s kg %4s gold/kg\n"
#define FORMAT_MAPA_MERCAT     "%-15s %12s gold\n"
#define MSG_NO_ATRACAT "You must be docked at an island.\n"

#define FORMAT_PETICIO_COMPRA "%s&%d"
#define MSG_COMPRA_FETA "Bought %d kg of %s for %d gold.\n"
#define MSG_QUANTITAT_INVALIDA "Error: amount must be positive.\n"
#define MSG_PRODUCTE_DESCONEGUT "Error: unknown product.\n"
#define MSG_OR_INSUFICIENT "Error: not enough gold.\n"
#define MSG_MAPA_PENDENT "Map purchase is not available yet.\n"

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
* @Finalitat: Guarda al navegant el viatge rebut a la resposta d'ACCEPT.
* @Parametres:  in/out: navegant = estat del navegant.
*               in: dades = OK&<VoyageID>&<Object>&<Destination>&<Reward>.
* @Retorn: TRAMA_OK si s'ha guardat, TRAMA_ERROR si el format es incorrecte
*          o no hi ha memoria.
*
************************************************/
int guardarViatge (Navegant *navegant, char *dades);

/***********************************************
*
* @Finalitat: Demana a Ithaca que assigni un viatge al navegant.
* @Parametres:  in/out: navegant = estat del navegant.
*               in: id_viatge = identificador del viatge escrit per l'usuari.
* @Retorn: ----.
*
************************************************/
void ferAccept (Navegant *navegant, char *id_viatge);

/***********************************************
*
* @Finalitat: Activar la bandera de racio pendent en rebre SIGALRM.
* @Parametres:  in: senyal = identificador de la senyal rebuda.
* @Retorn: ----.
*
************************************************/
void handleAlarma (int senyal);

/***********************************************
*
* @Finalitat: Abandona la ubicacio actual: s'acomiada d'Ithaca o de l'illa
*             si hi esta connectat i tanca la connexio.
* @Parametres:  in/out: navegant = estat del navegant.
* @Retorn: ----.
*
************************************************/
void marxarUbicacio (Navegant *navegant);

/***********************************************
*
* @Finalitat: Deixa el navegant atracat i comprova si es la destinacio del
*             seu viatge.
* @Parametres:  in/out: navegant = estat del navegant.
*               in: format = missatge d'acces al port a mostrar.
* @Retorn: ----.
*
************************************************/
void atracar (Navegant *navegant, char *format);

/***********************************************
*
* @Finalitat: Espera davant del port fins rebre DOCKED, consumint una racio
*             cada 5 segons d'espera sense espera activa.
* @Parametres:  in/out: navegant = estat del navegant.
* @Retorn: FINAL_SI si s'han acabat les provisions o s'ha premut CTRL+C,
*          FINAL_NO si no.
*
************************************************/
int esperarPort (Navegant *navegant);

/***********************************************
*
* @Finalitat: Anuncia l'arribada a l'illa i atraca o espera segons la
*             resposta.
* @Parametres:  in/out: navegant = estat del navegant.
* @Retorn: FINAL_SI si l'Odysseus ha d'acabar, FINAL_NO si no.
*
************************************************/
int arribarIlla (Navegant *navegant);

/***********************************************
*
* @Finalitat: Torna a connectar el navegant amb l'illa on es troba despres
*             d'haver-ne perdut la connexio.
* @Parametres:  in/out: navegant = estat del navegant.
* @Retorn: FINAL_SI si l'Odysseus ha d'acabar, FINAL_NO si no.
*
************************************************/
int tornarAIlla (Navegant *navegant);

/***********************************************
*
* @Finalitat: Navega fins a una illa connectada amb la ubicacio actual.
* @Parametres:  in/out: navegant = estat del navegant.
*               in: illa = nom de l'illa escrit per l'usuari.
* @Retorn: FINAL_SI si l'Odysseus ha d'acabar, FINAL_NO si no.
*
************************************************/
int ferSail (Navegant *navegant, char *illa);

/***********************************************
*
* @Finalitat: Mostra la capcalera del mercat amb el nom de l'illa en majuscules.
* @Parametres:  in: illa = nom de l'illa.
* @Retorn: ----.
*
************************************************/
void mostrarCapcaleraMercat (char *illa);

/***********************************************
*
* @Finalitat: Mostra una linia del mercat: un producte o el mapa.
* @Parametres:  in: nom = nom del producte.
*               in: quantitat = quilograms disponibles.
*               in: preu = preu per quilogram, o preu del mapa.
* @Retorn: ----.
*
************************************************/
void mostrarProducte (char *nom, char *quantitat, char *preu);

/***********************************************
*
* @Finalitat: Demana el mercat a l'illa i en llegeix totes les trames. Pot
*             mostrar-lo i buscar-hi el preu d'un producte.
* @Parametres:  in/out: navegant = estat del navegant.
*               in: mostrar = MOSTRAR_SI per mostrar el mercat, MOSTRAR_NO si no.
*               in: producte = producte del qual es vol el preu, o NULL.
*               out: preu = preu del producte, si s'ha trobat.
* @Retorn: RESPOSTA_OK, RESPOSTA_ERROR o RESPOSTA_PERDUDA.
*
************************************************/
int rebreMercat (Navegant *navegant, int mostrar, char *producte, int *preu);

/***********************************************
*
* @Finalitat: Mostra el mercat de l'illa on esta atracat el navegant.
* @Parametres:  in/out: navegant = estat del navegant.
* @Retorn: ----.
*
************************************************/
void ferListMarket (Navegant *navegant);


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

/***********************************************
*
* @Finalitat: Comprar un aliment i actualitzar la carrega i els diners.
* @Parametres: in/out: navegant = estat actual del navegant.
*              in: producte = nom de l'aliment demanat.
*              in: text_quantitat = quantitat numerica validada pel parser.
* @Retorn: ----.
*
************************************************/
void ferBuy (Navegant *navegant, char *producte, char *text_quantitat);

#endif