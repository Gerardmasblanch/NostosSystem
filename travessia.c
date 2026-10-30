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

int mostrarViatge (char *dades, int *total) {
    char **trossos = NULL;
    char *missatge = NULL;
    int n_trossos;
    int resultat = TRAMA_ERROR;

    trossos = splitString(dades, '&', &n_trossos);

    if (n_trossos == 6) {
        *total = atoi(trossos[1]);

        asprintf(&missatge, FORMAT_VIATGE, trossos[2], trossos[3], trossos[4], trossos[5]);
        printF(1, missatge);
        free(missatge);

        resultat = TRAMA_OK;
    }

    freeTokens(trossos, n_trossos);

    return resultat;
}

void ferListVoyages (Navegant *navegant) {
    char *dades = NULL;
    char *missatge = NULL;
    int resultat;
    int rebudes = 0;
    int total = 1;

    if (navegant->estat != ESTAT_CONNECTAT_ITHACA) {
        printF(1, MSG_NO_CONNECTAT_ITHACA);
        return;
    }

    enviarTrama(navegant->fd_connexio, TIPUS_LIST_VOYAGES, FLAG_PETICIO, "");
    printF(1, MSG_CAPCALERA_VIATGES);

    while (rebudes < total) {
        resultat = rebreResposta(navegant, TIPUS_LIST_VOYAGES, &dades);

        if (resultat == RESPOSTA_PERDUDA) {
            rebudes = total;
        } else if (resultat == RESPOSTA_ERROR) {
            asprintf(&missatge, MSG_ERROR_RESPOSTA, dades);
            printF(1, missatge);
            free(missatge);
            rebudes = total;
        } else if (strcmp(dades, TEXT_SENSE_VIATGES) == 0) {
            printF(1, MSG_SENSE_VIATGES);
            rebudes = total;
        } else if (mostrarViatge(dades, &total) == TRAMA_OK) {
            rebudes++;
        } else {
            printF(1, MSG_RESPOSTA_INCORRECTA);
            rebudes = total;
        }

        free(dades);
        dades = NULL;
    }
}

int guardarViatge (Navegant *navegant, char *dades) {
    char **trossos = NULL;
    int n_trossos;
    int resultat = TRAMA_ERROR;

    trossos = splitString(dades, '&', &n_trossos);

    if (n_trossos == 5) {
        navegant->viatge.object = copiarText(trossos[2]);
        navegant->viatge.destination = copiarText(trossos[3]);

        if (navegant->viatge.object != NULL && navegant->viatge.destination != NULL) {
            navegant->viatge.id = atoi(trossos[1]);
            navegant->viatge.reward = atoi(trossos[4]);
            navegant->te_viatge = AMB_VIATGE;
            resultat = TRAMA_OK;
        } else {
            printF(1, MSG_SENSE_MEMORIA);
            free(navegant->viatge.object);
            free(navegant->viatge.destination);
            navegant->viatge.object = NULL;
            navegant->viatge.destination = NULL;
        }
    }

    freeTokens(trossos, n_trossos);

    return resultat;
}

void ferAccept (Navegant *navegant, char *id_viatge) {
    char *dades = NULL;
    char *missatge = NULL;
    int resultat;

    if (navegant->estat != ESTAT_CONNECTAT_ITHACA) {
        printF(1, MSG_NO_CONNECTAT_ITHACA);
        return;
    }

    if (navegant->te_viatge == AMB_VIATGE) {
        printF(1, MSG_JA_TE_VIATGE);
        return;
    }

    enviarTrama(navegant->fd_connexio, TIPUS_ACCEPT, FLAG_PETICIO, id_viatge);
    resultat = rebreResposta(navegant, TIPUS_ACCEPT, &dades);

    if (resultat == RESPOSTA_ERROR) {
        asprintf(&missatge, MSG_ERROR_RESPOSTA, dades);
        printF(1, missatge);
        free(missatge);
    } else if (resultat == RESPOSTA_OK) {
        if (guardarViatge(navegant, dades) == TRAMA_OK) {
            asprintf(&missatge, MSG_VIATGE_ACCEPTAT, navegant->viatge.id, navegant->viatge.destination);
            printF(1, missatge);
            free(missatge);
        } else {
            printF(1, MSG_RESPOSTA_INCORRECTA);
        }
    }

    free(dades);
}


int executarComanda (Navegant *navegant, char *linea) {
    char **trossos = NULL;
    int n_trossos;
    int codi;

    codi = parseCommand(linea);
    trossos = splitString(linea, ' ', &n_trossos);

    if (codi == CMD_DESCONEGUDA) {
        printF(1, MSG_DESCONEGUDA);
    } else if (codi == CMD_CONNECT_ITHACA) {
        ferConnectIthaca(navegant);
    } else if (codi == CMD_LIST_VOYAGES) {
        ferListVoyages(navegant);
    } else if (codi == CMD_ACCEPT) {
        ferAccept(navegant, trossos[1]);
    } else if (codi != CMD_ERROR_SINTAXI) {
        printF(1, MSG_COMANDA_OK);
    }

    freeTokens(trossos, n_trossos);

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
