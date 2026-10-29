/***********************************************
*
* @Proposit: Implementacio de la travessia de l'Odysseus: bucle principal i
*            comandes del navegant.
* @Autor/s: Arnau Ricart i Gerard Mas
* @Data creacio: 29/10/2026
* @Data ultima modificacio: 29/10/2026
*
************************************************/

//Include del fitxer .h del mateix modul
#include "travessia.h"

//Procediments i funcions

int prepararSetOdysseus (fd_set *set, Navegant *navegant) {
    int max_fd = 0;

    FD_ZERO(set);

    FD_SET(0, set);

    if (navegant->fd_connexio != ERR_CODI) {
        FD_SET(navegant->fd_connexio, set);
        max_fd = navegant->fd_connexio;
    }

    return max_fd;
}

void perdreConnexio (Navegant *navegant) {
    char *missatge = NULL;

    asprintf(&missatge, MSG_CONNEXIO_PERDUDA, navegant->ubicacio);
    printF(1, missatge);
    free(missatge);

    close(navegant->fd_connexio);
    navegant->fd_connexio = ERR_CODI;

    if (strcmp(navegant->ubicacio, TEXT_ITHACA) == 0) {
        navegant->estat = ESTAT_ITHACA;
    } else {
        navegant->estat = ESTAT_DESCONNECTAT;
    }
}

void atendreServidor (Navegant *navegant) {
    char *dades = NULL;
    int tipus;
    int flags;

    if (rebreTrama(navegant->fd_connexio, &tipus, &flags, &dades) == TRAMA_ERROR) {
        perdreConnexio(navegant);
        printF(1, TEXT_PROMPT);
    }

    free(dades);
}

int rebreResposta (Navegant *navegant, int tipus_esperat, char **dades) {
    int tipus;
    int flags;
    int resultat = RESPOSTA_ERROR;

    if (rebreTrama(navegant->fd_connexio, &tipus, &flags, dades) == TRAMA_ERROR) {
        perdreConnexio(navegant);
        resultat = RESPOSTA_PERDUDA;
    } else if (tipus == tipus_esperat && flags == FLAG_OK) {
        resultat = RESPOSTA_OK;
    }

    return resultat;
}

void ferConnectIthaca (Navegant *navegant) {
    char *dades = NULL;
    char *missatge = NULL;
    int resultat;

    if (navegant->estat == ESTAT_CONNECTAT_ITHACA) {
        printF(1, MSG_JA_CONNECTAT);
        return;
    }

    if (navegant->estat != ESTAT_ITHACA) {
        printF(1, MSG_NOMES_A_ITHACA);
        return;
    }

    navegant->fd_connexio = connectarServidor(navegant->config.ithaca_ip, navegant->config.ithaca_port);

    if (navegant->fd_connexio == ERR_CODI) {
        asprintf(&missatge, MSG_ERROR_CONNEXIO, TEXT_ITHACA);
        printF(1, missatge);
        free(missatge);
        return;
    }

    enviarTrama(navegant->fd_connexio, TIPUS_CONNECT, FLAG_PETICIO, navegant->config.name);
    resultat = rebreResposta(navegant, TIPUS_CONNECT, &dades);

    if (resultat == RESPOSTA_OK) {
        navegant->estat = ESTAT_CONNECTAT_ITHACA;

        asprintf(&missatge, MSG_CONNECTAT, TEXT_ITHACA);
        printF(1, missatge);
        free(missatge);

    } else if (resultat == RESPOSTA_ERROR) {
        asprintf(&missatge, MSG_ERROR_RESPOSTA, dades);
        printF(1, missatge);
        free(missatge);

        close(navegant->fd_connexio);
        navegant->fd_connexio = ERR_CODI;
    }

    free(dades);
}



int executarComanda (Navegant *navegant, char *linea) {
    int codi;

    codi = parseCommand(linea);

    if (codi == CMD_DESCONEGUDA) {
        printF(1, MSG_DESCONEGUDA);
    } else if(codi == CMD_CONNECT_ITHACA) {
        ferConnectIthaca(navegant);
    } else if (codi != CMD_ERROR_SINTAXI) {
        printF(1, MSG_COMANDA_OK);
    }

    return FINAL_NO;
}

int atendreTeclat (Navegant *navegant) {
    char *linea = NULL;
    int final = FINAL_SI;

    linea = readUntil(0, '\n');

    if (linea != NULL) {
        final = executarComanda(navegant, linea);
        free(linea);

        if (final == FINAL_NO) {
            printF(1, "\n");
            printF(1, TEXT_PROMPT);
        }
    }

    return final;
}

void executarOdysseus (Navegant *navegant, int *sortir) {

    fd_set set;
    int max_fd;
    int final = FINAL_NO;

    printF(1, TEXT_PROMPT);

    while (*sortir == 0 && final == FINAL_NO) {
        max_fd = prepararSetOdysseus(&set, navegant);

        if (select(max_fd + 1, &set, NULL, NULL, NULL) > 0) {
            if (navegant->fd_connexio != ERR_CODI && FD_ISSET(navegant->fd_connexio, &set)) {
                atendreServidor(navegant);
            }
            if (FD_ISSET(0, &set)) {
                final = atendreTeclat(navegant);
            }
        }
    }
}
