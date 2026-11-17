/***********************************************
*
* @Proposit: Travessia de l'Odysseus, bucle principal amb select i execucio
*            de les comandes del navegant.
* @Autor/s: Arnau Ricart i Gerard Mas
* @Data creacio: 29/10/2026
* @Data ultima modificacio: 17/11/2026
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
#include "fitxer.h"

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
#define MSG_COMANDA_OK  "Command Available in Phase 3\n"
#define MSG_CONNEXIO_PERDUDA "\nLost connection with %s.\n"
#define MSG_ERROR_PIPE       "Error: the alert pipe could not be created.\n"
#define MSG_CONNECTAT      "Connected to %s.\n"
#define MSG_ERROR_CONNEXIO "Error: could not reach %s.\n"
#define MSG_ERROR_RESPOSTA "Error: %s.\n"
#define MSG_JA_CONNECTAT   "You are already connected to Ithaca.\n"
#define MSG_NOMES_A_ITHACA "You can only connect to Ithaca from Ithaca.\n"
#define MSG_TORNANT_ITHACA "Returning directly to Ithaca...\n"
#define TEXT_SENSE_VIATGES "0"
#define MSG_CAPCALERA_VIATGES "\n--- AVAILABLE VOYAGES ---\n\n"
#define FORMAT_VIATGE "%s. %-20s -> %-12s [%s]\n"
#define MSG_SENSE_VIATGES "No voyages available.\n"
#define MSG_NO_CONNECTAT_ITHACA "You must be connected to Ithaca.\n"
#define MSG_RESPOSTA_INCORRECTA "Error: unexpected response from the server.\n"
#define MSG_VIATGE_ACCEPTAT "Voyage %d accepted.\nDestination: %s.\n"
#define MSG_JA_TE_VIATGE    "You already have a voyage assigned.\n"
#define MSG_OBJECTE_REBUT   "Object received: %s.\n"
#define MSG_ERROR_OBJECTE   "Error: the object could not be received. The voyage has been cancelled.\n"
#define FORMAT_PETICIO_DELIVER  "%d&%s"
#define MSG_LLIURANT            "Delivering %s to %s...\n"
#define MSG_OBJECTE_TRANSFERIT  "Object transferred correctly.\n"
#define MSG_SEGELLANT           "\n%s is stamping the object...\n"
#define MSG_SEGELLAT_REBUT      "\nStamped object received.\n"
#define MSG_LLIURAMENT_FET      "\nDelivery completed.\nThe mark of %s has been obtained.\n"
#define MSG_SENSE_VIATGE_DELIVER "You don't have any object to deliver.\n"
#define MSG_JA_LLIURAT          "The object has already been delivered. Return to Ithaca and CLAIM the reward.\n"
#define MSG_NO_ES_DESTI         "You must be docked at %s, the destination of your voyage.\n"
#define MSG_ERROR_LLIURAMENT    "Error: the delivery could not be completed.\n"
#define FORMAT_PETICIO_CLAIM   "%d"
#define TEXT_CLAIM_OK          "OK"
#define N_CAMPS_CLAIM          2
#define MSG_PRESENTANT         "Presenting the stamped %s...\n"
#define MSG_CLAIM_FET          "Glyph of %s verified.\n\nVoyage %d completed.\nReward received: %d gold.\n"
#define MSG_SENSE_LLIURAMENT   "You have no delivered voyage to claim.\n"
#define MSG_ERROR_CLAIM        "Error: the claim could not be completed.\n"
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
#define MSG_ESPERANT_PORT     "You are waiting to enter the port.\n"
#define MSG_SENSE_VIATGE_SAIL "You must accept a voyage before leaving Ithaca.\n"
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
#define MSG_CAL_LIST_MARKET "Error: check the market first with LIST MARKET.\n"

#define FORMAT_PETICIO_VENDA "%s&%d"
#define MSG_VENDA_FETA "Sold %d kg of %s for %d gold.\n"
#define MSG_CARREGA_INSUFICIENT "Error: not enough food in cargo.\n"
#define MSG_NO_EL_PORTES "Error: you are not carrying %s.\n"

#define MAPA_PREU      50
#define MAPA_QUANTITAT 1

#define MSG_NOMES_UN_MAPA       "Error: maps can only be bought one at a time (BUY MAP 1).\n"
#define MSG_OR_INSUFICIENT_MAPA "Error: a map costs %d gold and you have %d.\n"
#define MSG_MAPA_COMPRAT        "Bought the map of %s for %d gold.\n"
#define MSG_RUTES_NOVES         "New routes have been charted.\n"
#define MSG_SENSE_RUTES         "No sea routes leave this island.\n"
#define MSG_ERROR_MAPA_TERMINAL "Error: the terminal must be at least 80 columns wide.\n"
#define MSG_ERROR_MAPA          "Error: the map could not be displayed.\n"

//Procediments i funcions

/***********************************************
*
* @Finalitat: Omple el conjunt de descriptors amb el teclat i
*             la connexio actual (si n'hi ha).
* @Parametres:  out: set = conjunt de descriptors a omplir.
*               in: navegant = navegant amb la connexio actual.
* @Retorn: El descriptor mes gran del conjunt.
*
************************************************/
int TRAVESSIA_prepararSetOdysseus (fd_set *set, Navegant *navegant);

/***********************************************
*
* @Finalitat: Tanca la connexio actual perque s'ha perdut i actualitza
*             l'estat del navegant segons on es trobi.
* @Parametres:  in/out: navegant = navegant amb la connexio actual.
* @Retorn: ----.
*
************************************************/
void TRAVESSIA_perdreConnexio (Navegant *navegant);

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
int TRAVESSIA_rebreResposta (Navegant *navegant, int tipus_esperat, char **dades);


/***********************************************
*
* @Finalitat: Atén una trama no esperada del servidor. Si la connexio s'ha
*             perdut, la tanca i actualitza l'estat del navegant.
* @Parametres:  in/out: navegant = navegant amb la connexio actual.
* @Retorn: ----.
*
************************************************/
void TRAVESSIA_atendreServidor (Navegant *navegant);


/***********************************************
*
* @Finalitat: Connecta el navegant amb Ithaca i s'hi identifica amb el seu nom.
* @Parametres:  in/out: navegant = estat del navegant.
* @Retorn: ----.
*
************************************************/
void TRAVESSIA_ferConnectIthaca (Navegant *navegant);

/***********************************************
*
* @Finalitat: Despres del DELIVER, abandona l'illa de desti i torna directament a Ithaca sense passar per illes intermedies, encara sense connectar-s'hi. La fan servir SAIL Ithaca i CONNECT ITHACA.
* @Parametres:  in/out: navegant = estat del navegant, amb el viatge ja lliurat.
* @Retorn: 0 si ja es a Ithaca, ERR_CODI si no hi ha memoria.
*
************************************************/
int TRAVESSIA_tornarAIthaca (Navegant *navegant);


/***********************************************
*
* @Finalitat: Interpreta una trama de LIST VOYAGES i mostra el viatge.
* @Parametres:  in: dades = <Index>&<Total>&<VoyageID>&<Object>&<Destination>&<Reward>.
*               out: total = nombre total de viatges que enviara Ithaca.
* @Retorn: TRAMA_OK si el format es correcte, TRAMA_ERROR si no.
*
************************************************/
int TRAVESSIA_mostrarViatge (char *dades, int *total);

/***********************************************
*
* @Finalitat: Demana a Ithaca els viatges disponibles i els mostra.
* @Parametres:  in/out: navegant = estat del navegant.
* @Retorn: ----.
*
************************************************/
void TRAVESSIA_ferListVoyages (Navegant *navegant);

/***********************************************
*
* @Finalitat: Guarda al navegant el viatge rebut a la resposta d'ACCEPT.
* @Parametres:  in/out: navegant = estat del navegant.
*               in: dades = OK&<VoyageID>&<Object>&<Destination>&<Reward>.
* @Retorn: TRAMA_OK si s'ha guardat, TRAMA_ERROR si el format es incorrecte
*          o no hi ha memoria.
*
************************************************/
int TRAVESSIA_guardarViatge (Navegant *navegant, char *dades);

/***********************************************
*
* @Finalitat: Demana a Ithaca que assigni un viatge al navegant.
* @Parametres:  in/out: navegant = estat del navegant.
*               in: id_viatge = identificador del viatge escrit per l'usuari.
* @Retorn: ----.
*
************************************************/
void TRAVESSIA_ferAccept (Navegant *navegant, char *id_viatge);

/***********************************************
*
* @Finalitat: Rep l'objecte del viatge que Ithaca envia despres de l'ACCEPT i el desa a la carpeta del navegant. Si falla, mostra l'error i esborra el viatge; nomes tanca la connexio amb Ithaca si l'error es de comunicacio (amb un error de mida o MD5 hi continua connectat i pot acceptar un altre viatge).
* @Parametres:  in/out: navegant = estat del navegant, amb el viatge ja guardat.
* @Retorn: ----.
*
************************************************/
void TRAVESSIA_rebreObjecte (Navegant *navegant);

/***********************************************
*
* @Finalitat: Comprova que es pot fer el DELIVER: atracat a l'illa de desti, amb un viatge actiu amb objecte i sense haver-lo lliurat encara. Si no es pot, mostra el motiu.
* @Parametres:  in: navegant = estat del navegant.
* @Retorn: TRAMA_OK si es pot lliurar, TRAMA_ERROR si no.
*
************************************************/
int TRAVESSIA_comprovarDeliver (Navegant *navegant);

/***********************************************
*
* @Finalitat: Envia l'objecte a l'illa (DELIVERY) i rep la imatge segellada (SIGNED) amb el prefix signed_.
* @Parametres:  in: navegant = estat del navegant, connectat a l'illa de desti.
*               out: path_segellat = ruta de la imatge segellada reservada amb malloc si te exit, NULL si no.
* @Retorn: FITXER_OK si les dues transferencies es completen; FITXER_ERROR si cal tancar la connexio; un altre codi FITXER_ si l'illa encara respondra el DELIVER amb error.
*
************************************************/
int TRAVESSIA_transferirLliurament (Navegant *navegant, char **path_segellat);

/***********************************************
*
* @Finalitat: Rep la resposta final del DELIVER. Si tot ha anat be, guarda la imatge segellada i marca el viatge com a lliurat; si no, mostra l'error i allibera la ruta.
* @Parametres:  in/out: navegant = estat del navegant.
*               in: resultat_fitxer = resultat de les transferencies.
*               in: path_segellat = ruta de la imatge segellada o NULL.
* @Retorn: ----.
*
************************************************/
void TRAVESSIA_acabarDeliver (Navegant *navegant, int resultat_fitxer, char *path_segellat);

/***********************************************
*
* @Finalitat: Executa la comanda DELIVER: lliura l'objecte a l'illa de desti i rep la imatge segellada que servira de comprovant a Ithaca.
* @Parametres:  in/out: navegant = estat del navegant.
* @Retorn: ----.
*
************************************************/
void TRAVESSIA_ferDeliver (Navegant *navegant);

/***********************************************
*
* @Finalitat: Comprova que es pot fer el CLAIM: connectat a Ithaca i amb el viatge actiu ja lliurat i la imatge segellada. Si no es pot, mostra el motiu.
* @Parametres:  in: navegant = estat del navegant.
* @Retorn: TRAMA_OK si es pot reclamar, TRAMA_ERROR si no.
*
************************************************/
int TRAVESSIA_comprovarClaim (Navegant *navegant);

/***********************************************
*
* @Finalitat: Valida la resposta OK&Reward del CLAIM, suma la recompensa a l'or, mostra el resultat i esborra el viatge completat.
* @Parametres:  in/out: navegant = estat del navegant.
*               in: dades = dades de la resposta correcta d'Ithaca.
* @Retorn: ----.
*
************************************************/
void TRAVESSIA_cobrarRecompensa (Navegant *navegant, char *dades);

/***********************************************
*
* @Finalitat: Executa la comanda CLAIM: envia a Ithaca la imatge segellada amb el nom original de l'objecte i, si la valida, cobra la recompensa.
* @Parametres:  in/out: navegant = estat del navegant.
* @Retorn: ----.
*
************************************************/
void TRAVESSIA_ferClaim (Navegant *navegant);

/***********************************************
*
* @Finalitat: Abandona la ubicacio actual, s'acomiada d'Ithaca o de l'illa
*             si hi esta connectat i tanca la connexio.
* @Parametres:  in/out: navegant = estat del navegant.
* @Retorn: ----.
*
************************************************/
void TRAVESSIA_marxarUbicacio (Navegant *navegant);

/***********************************************
*
* @Finalitat: En sortir de l'Odysseus (CTRL+C o final de l'entrada), s'acomiada d'Ithaca (0x05) o de l'illa on esta atracat (0x16), n'espera la resposta sense mirar la pipe d'avis i tanca la connexio.
* @Parametres:  in/out: navegant = estat del navegant.
* @Retorn: ----.
*
************************************************/
void TRAVESSIA_acomiadarSortida (Navegant *navegant);

/***********************************************
*
* @Finalitat: Deixa el navegant atracat i comprova si es la destinacio del
*             seu viatge.
* @Parametres:  in/out: navegant = estat del navegant.
*               in: format = missatge d'acces al port a mostrar.
* @Retorn: ----.
*
************************************************/
void TRAVESSIA_atracar (Navegant *navegant, char *format);

/***********************************************
*
* @Finalitat: Consumeix una racio mentre el navegant espera davant del port
*             i torna a programar l'alarma. Si no queden provisions, el
*             navegant es perd al mar.
* @Parametres:  in/out: navegant = estat del navegant.
* @Retorn: FINAL_SI si s'han acabat les provisions, FINAL_NO si no.
*
************************************************/
int TRAVESSIA_gestionarRacio (Navegant *navegant);

/***********************************************
*
* @Finalitat: Anuncia l'arribada a l'illa i atraca o comenca a esperar
*             segons la resposta.
* @Parametres:  in/out: navegant = estat del navegant.
* @Retorn: ----.
*
************************************************/
void TRAVESSIA_arribarIlla (Navegant *navegant);

/***********************************************
*
* @Finalitat: Torna a connectar el navegant amb l'illa on es troba despres
*             d'haver-ne perdut la connexio.
* @Parametres:  in/out: navegant = estat del navegant.
* @Retorn: ----.
*
************************************************/
void TRAVESSIA_tornarAIlla (Navegant *navegant);

/***********************************************
*
* @Finalitat: Navega fins a una illa connectada amb la ubicacio actual. Despres del DELIVER, SAIL Ithaca torna directament a Ithaca sense connectar-s'hi.
* @Parametres:  in/out: navegant = estat del navegant.
*               in: illa = nom de l'illa escrit per l'usuari.
* @Retorn: ----.
*
************************************************/
void TRAVESSIA_ferSail (Navegant *navegant, char *illa);

/***********************************************
*
* @Finalitat: Mostra la capcalera del mercat amb el nom de l'illa en majuscules.
* @Parametres:  in: illa = nom de l'illa.
* @Retorn: ----.
*
************************************************/
void TRAVESSIA_mostrarCapcaleraMercat (char *illa);

/***********************************************
*
* @Finalitat: Mostra una linia del mercat, un producte o el mapa.
* @Parametres:  in: nom = nom del producte.
*               in: quantitat = quilograms disponibles.
*               in: preu = preu per quilogram, o preu del mapa.
* @Retorn: ----.
*
************************************************/
void TRAVESSIA_mostrarProducte (char *nom, char *quantitat, char *preu);

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
int TRAVESSIA_rebreMercat (Navegant *navegant, int mostrar, char *producte, int *preu);

/***********************************************
*
* @Finalitat: Mostra el mercat de l'illa on esta atracat el navegant.
* @Parametres:  in/out: navegant = estat del navegant.
* @Retorn: ----.
*
************************************************/
void TRAVESSIA_ferListMarket (Navegant *navegant);


/***********************************************
*
* @Finalitat: Interpreta una linia del teclat i executa la comanda.
* @Parametres:  in/out: navegant = estat del navegant.
*               in: linea = linia escrita per l'usuari.
* @Retorn: ----.
*
************************************************/
void TRAVESSIA_executarComanda (Navegant *navegant, char *linea);

/***********************************************
*
* @Finalitat: Llegeix una linia del teclat i l'executa.
* @Parametres:  in/out: navegant = estat del navegant.
* @Retorn: FINAL_SI si s'ha tancat l'entrada, FINAL_NO si no.
*
************************************************/
int TRAVESSIA_atendreTeclat (Navegant *navegant);

/***********************************************
*
* @Finalitat: Bucle principal de l'Odysseus, atén el teclat, la connexio i
*             les racions mentre espera port, fins que s'activa la bandera
*             de sortida o s'ha d'acabar.
* @Parametres:  in/out: navegant = estat del navegant.
*               in: sortir = bandera que posa a 1 el handler de SIGINT.
*               in/out: racio_pendent = bandera que posa a 1 el handler de
*                       SIGALRM.
* @Retorn: ----.
*
************************************************/
void TRAVESSIA_executarOdysseus (Navegant *navegant, int *sortir, int *racio_pendent);

/***********************************************
*
* @Finalitat: Comprar un aliment i actualitzar la carrega i els diners.
* @Parametres: in/out: navegant = estat actual del navegant.
*              in: producte = nom de l'aliment demanat.
*              in: text_quantitat = quantitat numerica validada pel parser.
* @Retorn: ----.
*
************************************************/
void TRAVESSIA_ferBuy (Navegant *navegant, char *producte, char *text_quantitat);

/***********************************************
*
* @Finalitat: Vendre un aliment i actualitzar la carrega i els diners.
* @Parametres: in/out: navegant = estat actual del navegant.
*              in: producte = nom de l'aliment que es ven.
*              in: text_quantitat = quantitat numerica validada pel parser.
* @Retorn: ----.
*
************************************************/
void TRAVESSIA_ferSell (Navegant *navegant, char *producte, char *text_quantitat);

/***********************************************
*
* @Finalitat: Compra el mapa de l'illa i afegeix les seves rutes al mapa
*             conegut.
* @Parametres:  in/out: navegant = estat del navegant.
*               in: quantitat = quantitat escrita per l'usuari.
* @Retorn: ----.
*
************************************************/
void TRAVESSIA_ferBuyMap (Navegant *navegant, char *quantitat);

/***********************************************
*
* @Finalitat: Mostra amb SPHRAGIS el mapa i les rutes conegudes.
* @Parametres:  in: navegant = navegant amb el mapa.
* @Retorn: ----.
*
************************************************/
void TRAVESSIA_ferMap (Navegant *navegant);

#endif