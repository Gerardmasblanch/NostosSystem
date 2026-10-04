/***********************************************
*
* @Proposit: Implementacio del reconeixement i la validacio sintactica de les
*            comandes del terminal interactiu del proces Odysseus.
* @Autor/s: Arnau Ricart i Gerard Mas
* @Data creacio: 25/09/2026
* @Data ultima modificacio: 25/09/2026
*
************************************************/

//Include del .h
#include "commands.h"

//Procediments i funcions
int parseSenseArguments (int n_trossos, char *us, int codi_valid) {
    char *missatge;

    if (n_trossos != 1) {
        asprintf(&missatge, "Usage: %s\n", us);
        printF(1, missatge);
        free(missatge);
        return CMD_ERROR_SINTAXI;
    }

    return codi_valid;
}


int parseAmbQuantitat (char **trossos, int n_trossos, char *us, int codi_valid) {
    char *missatge;

    if (n_trossos != 3 || parseInt(trossos[2]) == VALOR_NO_NUMERIC) {
        asprintf(&missatge, "Usage: %s <product> <amount>\n", us);
        printF(1, missatge);
        free(missatge);
        return CMD_ERROR_SINTAXI;
    }

    return codi_valid;
}


int parseCommand (char *linea) {
    
    char **trossos;
    int n_trossos;
    int codi;

    trossos = splitString(linea, ' ', &n_trossos);

    if (n_trossos == 0) {
        freeTokens(trossos,n_trossos);
        codi = CMD_DESCONEGUDA;
        return codi;
    }
    if(strcasecmp(trossos[0],"MAP") == 0){
        codi = parseSenseArguments(n_trossos, "MAP", CMD_MAP);
    } else if(strcasecmp(trossos[0],"STATUS") == 0) {
        codi = parseSenseArguments(n_trossos, "STATUS", CMD_STATUS);
    } else if(strcasecmp(trossos[0],"DELIVER") == 0) {
        codi = parseSenseArguments(n_trossos, "DELIVER", CMD_DELIVER);
    } else if(strcasecmp(trossos[0],"CLAIM") == 0) {
        codi = parseSenseArguments(n_trossos, "CLAIM", CMD_CLAIM);
    } else if(strcasecmp(trossos[0],"BUY") == 0) {
        codi = parseAmbQuantitat(trossos, n_trossos, "BUY", CMD_BUY);
    } else if(strcasecmp(trossos[0],"SELL") == 0) {
        codi = parseAmbQuantitat(trossos, n_trossos, "SELL", CMD_SELL);
    } else if(strcasecmp(trossos[0],"CONNECT") == 0) {
        if (n_trossos == 2 && strcasecmp(trossos[1],"ITHACA") == 0) {
            codi = CMD_CONNECT_ITHACA;
        } else {
            printF(1, "Usage: CONNECT ITHACA\n");
            codi = CMD_ERROR_SINTAXI;
        }
    } else if(strcasecmp(trossos[0],"LIST") == 0) {
        if (n_trossos == 2 && strcasecmp(trossos[1],"VOYAGES") == 0) {
            codi = CMD_LIST_VOYAGES;
        } else if (n_trossos == 2 && strcasecmp(trossos[1],"MARKET") == 0) {
            codi = CMD_LIST_MARKET;
        } else {
            printF(1, "Usage: LIST <VOYAGES|MARKET>\n");
            codi = CMD_ERROR_SINTAXI;
        }
    } else if(strcasecmp(trossos[0],"ACCEPT") == 0) {
        if (n_trossos == 2 && parseInt(trossos[1]) != VALOR_NO_NUMERIC) {
            codi = CMD_ACCEPT;
        } else {
            printF(1, "Usage: ACCEPT <voyage_id>\n");
            codi = CMD_ERROR_SINTAXI;
        }
    } else if(strcasecmp(trossos[0],"SAIL") == 0) {
        if (n_trossos == 2) {
            codi = CMD_SAIL;
        } else {
            printF(1, "Usage: SAIL <island>\n");
            codi = CMD_ERROR_SINTAXI;
        }
    } else {
        codi = CMD_DESCONEGUDA;
    }
    freeTokens(trossos, n_trossos);
    return codi;    
}