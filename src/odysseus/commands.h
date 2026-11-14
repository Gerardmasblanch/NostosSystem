/***********************************************
*
* @Proposit: gestió de les comandes introduides per l'usuari a l'Odysseus.
* @Autor/s: Arnau Ricart i Gerard Mas
* @Data creacio: 25/09/2026
* @Data ultima modificacio: 25/09/2026
*
************************************************/


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
* @Finalitat: Valida una comanda que no admet arguments.
* @Parametres:  in: n_trossos = nombre de paraules introduides.
*               in: us = nom de la comanda, per al missatge de sintaxi.
*               in: codi_valid = identificador a retornar si es correcta.
* @Retorn: codi_valid, o CMD_ERROR_SINTAXI.
*
************************************************/

int parseSenseArguments(int n_trossos, char *us, int codi_valid);

/***********************************************
*
* @Finalitat: Valida una comanda de producte i quantitat (BUY i SELL).
* @Parametres:  in: trossos = paraules introduides.
*               in: n_trossos = nombre de paraules introduides.
*               in: us = nom de la comanda, per al missatge de sintaxi.
*               in: codi_valid = identificador a retornar si es correcta.
*
* @Retorn: codi_valid, o CMD_ERROR_SINTAXI.
*
************************************************/
int parseAmbQuantitat(char **trossos, int n_trossos, char *us, int codi_valid);

/***********************************************
*
* @Finalitat: Reconeix una comanda del terminal i en valida la sintaxi,
*             sense executar-ne la funcionalitat.
* @Parametres:  in: linia = text introduit per l'usuari.
* @Retorn: L'identificador de la comanda, CMD_ERROR_SINTAXI si els arguments
*          no son correctes, o CMD_DESCONEGUDA si no existeix.
*
************************************************/
int parseCommand (char *linia);

#endif
