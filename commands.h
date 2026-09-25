/***********************************************
*
* @Proposit: Reconeixement i validacio sintactica de les comandes del
*            terminal interactiu del proces Odysseus.
* @Autor/s: Arnau Ricart i Gerard Mas
* @Data creacio: 25/09/2026
* @Data ultima modificacio: 25/09/2026
*
************************************************/

//Define Guards
#ifndef _COMMANDS_H
#define _COMMANDS_H

//Llibreries del sistema
#include <strings.h>

//Llibreries propies
#include "utils.h"
#include "types.h"

//Constants: identificador de cada comanda reconeguda pel terminal.
#define CMD_DESCONEGUDA     0
#define CMD_ERROR_SINTAXI   1
#define CMD_CONNECT_ITHACA  2
#define CMD_LIST_VOYAGES    3
#define CMD_LIST_MARKET     4
#define CMD_ACCEPT          5
#define CMD_SAIL            6
#define CMD_MAP             7
#define CMD_BUY             8
#define CMD_SELL            9
#define CMD_STATUS          10
#define CMD_DELIVER         11
#define CMD_CLAIM           12

//Procediments i funcions

/***********************************************
*
* @Finalitat: Reconeix una comanda introduida pel terminal i en valida la
*             sintaxi i els arguments, sense executar-ne la funcionalitat.
* @Parametres:  in: linia = text introduit per l'usuari.
* @Retorn: L'identificador de la comanda reconeguda, CMD_ERROR_SINTAXI si la
*          comanda existeix pero els arguments no son correctes, o
*          CMD_DESCONEGUDA si no correspon a cap comanda.
*
************************************************/
int parseCommand (char *linia);

#endif
