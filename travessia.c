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

//Variables globals
int racio_pendent = 0;

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

    if (navegant->te_viatge == AMB_VIATGE) {
        printF(1, MSG_JA_TE_VIATGE);
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

void handleAlarma (int senyal) {
    if (senyal == SIGALRM) {
        racio_pendent = 1;
    }
}


void marxarUbicacio (Navegant *navegant) {
    char *dades = NULL;
    char *missatge = NULL;

    asprintf(&missatge, MSG_MARXANT, navegant->ubicacio);
    printF(1, missatge);
    free(missatge);

    if (navegant->estat == ESTAT_CONNECTAT_ITHACA) {
        enviarTrama(navegant->fd_connexio, TIPUS_SORTIR_ITHACA, FLAG_PETICIO, "");
        rebreResposta(navegant, TIPUS_SORTIR_ITHACA, &dades);
    } else if (navegant->estat == ESTAT_ATRACAT) {
        enviarTrama(navegant->fd_connexio, TIPUS_SORTIR_ILLA, FLAG_PETICIO, "");
        rebreResposta(navegant, TIPUS_SORTIR_ILLA, &dades);
    }

    free(dades);

    if (navegant->fd_connexio != ERR_CODI) {
        close(navegant->fd_connexio);
        navegant->fd_connexio = ERR_CODI;
    }
}

void atracar (Navegant *navegant, char *format) {
    char *missatge = NULL;

    navegant->estat = ESTAT_ATRACAT;

    asprintf(&missatge, format, navegant->ubicacio);
    printF(1, missatge);
    free(missatge);

    if (navegant->te_viatge == AMB_VIATGE && strcasecmp(navegant->ubicacio, navegant->viatge.destination) == 0) {
        asprintf(&missatge, MSG_DESTI, navegant->ubicacio);
        printF(1, missatge);
        free(missatge);
    }
}

int esperarPort (Navegant *navegant) {
    fd_set set;
    char *dades = NULL;
    char *missatge = NULL;
    int tipus;
    int flags;
    int resultat;
    int esperant = 1;
    int final = FINAL_NO;

    racio_pendent = 0;
    signal(SIGALRM, handleAlarma);
    alarm(SEGONS_RACIO);

    while (esperant == 1) {
        FD_ZERO(&set);
        FD_SET(navegant->fd_connexio, &set);

        resultat = 0;
        if (racio_pendent == 0) {
            resultat = select(navegant->fd_connexio + 1, &set, NULL, NULL, NULL);
        }

        if (racio_pendent == 1) {
            racio_pendent = 0;

            if (totalProvisions(navegant) <= RACIO_KG) {
                asprintf(&missatge, MSG_SENSE_PROVISIONS, navegant->config.name);
                printF(1, missatge);
                free(missatge);
                esperant = 0;
                final = FINAL_SI;
            } else {
                consumirRacio(navegant, RACIO_KG);
                asprintf(&missatge, MSG_RACIO, totalProvisions(navegant));
                printF(1, missatge);
                free(missatge);
                alarm(SEGONS_RACIO);
            }
        } else if (resultat < 0) {
            esperant = 0;
            final = FINAL_SI;
        } else if (rebreTrama(navegant->fd_connexio, &tipus, &flags, &dades) == TRAMA_ERROR) {
            perdreConnexio(navegant);
            esperant = 0;
        } else if (tipus == TIPUS_ARRIBADA && flags == FLAG_OK && strcmp(dades, TEXT_ATRACAT) == 0) {
            atracar(navegant, MSG_ACCES_ESPERA);
            esperant = 0;
        }

        free(dades);
        dades = NULL;
    }

    alarm(0);

    return final;
}

int arribarIlla (Navegant *navegant) {
    char *dades = NULL;
    char *missatge = NULL;
    int resultat;
    int final = FINAL_NO;

    printF(1, "\n");
    asprintf(&missatge, MSG_CONNECTAT, navegant->ubicacio);
    printF(1, missatge);
    free(missatge);

    enviarTrama(navegant->fd_connexio, TIPUS_ARRIBADA, FLAG_PETICIO, navegant->config.name);
    resultat = rebreResposta(navegant, TIPUS_ARRIBADA, &dades);

    if (resultat == RESPOSTA_OK && strcmp(dades, TEXT_ATRACAT) == 0) {
        atracar(navegant, MSG_ACCES);
    } else if (resultat == RESPOSTA_OK && strcmp(dades, TEXT_ESPERA) == 0) {
        printF(1, MSG_PORT_COMPLET);
        final = esperarPort(navegant);
    } else if (resultat != RESPOSTA_PERDUDA) {
        if (resultat == RESPOSTA_ERROR) {
            asprintf(&missatge, MSG_ERROR_RESPOSTA, dades);
            printF(1, missatge);
            free(missatge);
        } else {
            printF(1, MSG_RESPOSTA_INCORRECTA);
        }
        close(navegant->fd_connexio);
        navegant->fd_connexio = ERR_CODI;
    }

    free(dades);

    return final;
}

int tornarAIlla (Navegant *navegant) {
    char *missatge = NULL;
    char *ip = NULL;
    int port = 0;
    int index;

    if (strcasecmp(navegant->ubicacio, TEXT_AEAEA) == 0) {
        ip = navegant->config.aeaea_ip;
        port = navegant->config.aeaea_port;
    } else {
        index = buscarAdreca(navegant, navegant->ubicacio);
        if (index != ERR_CODI) {
            ip = navegant->adreces[index].ip;
            port = navegant->adreces[index].port;
        }
    }

    if (ip != NULL) {
        navegant->fd_connexio = connectarServidor(ip, port);
    }

    if (navegant->fd_connexio == ERR_CODI) {
        asprintf(&missatge, MSG_ERROR_CONNEXIO, navegant->ubicacio);
        printF(1, missatge);
        free(missatge);
        return FINAL_NO;
    }

    return arribarIlla(navegant);
}

int ferSail (Navegant *navegant, char *illa) {
    char *missatge = NULL;
    char *nom = NULL;
    char *ip = NULL;
    int port = 0;
    int index;
    int fd_nou;

    if (navegant->estat == ESTAT_DESCONNECTAT && strcasecmp(illa, navegant->ubicacio) == 0) {
        return tornarAIlla(navegant);
    }

    if (navegant->estat == ESTAT_ITHACA || navegant->estat == ESTAT_CONNECTAT_ITHACA) {
        if (strcasecmp(illa, TEXT_AEAEA) == 0) {
            nom = TEXT_AEAEA;
            ip = navegant->config.aeaea_ip;
            port = navegant->config.aeaea_port;
        }
    } else if (esRutaDirecta(navegant, illa) == RUTA_DIRECTA) {
        index = buscarAdreca(navegant, illa);
        if (index != ERR_CODI) {
            nom = navegant->adreces[index].name;
            ip = navegant->adreces[index].ip;
            port = navegant->adreces[index].port;
        }
    }

    if (nom == NULL) {
        asprintf(&missatge, MSG_NO_ES_POT_NAVEGAR, illa);
        printF(1, missatge);
        free(missatge);
        return FINAL_NO;
    }

    fd_nou = connectarServidor(ip, port);
    if (fd_nou == ERR_CODI) {
        asprintf(&missatge, MSG_ERROR_CONNEXIO, nom);
        printF(1, missatge);
        free(missatge);
        return FINAL_NO;
    }

    marxarUbicacio(navegant);

    if (canviarUbicacio(navegant, nom) == ERR_CODI) {
        close(fd_nou);
        navegant->estat = ESTAT_DESCONNECTAT;
        return FINAL_NO;
    }

    asprintf(&missatge, MSG_NAVEGANT, nom);
    printF(1, missatge);
    free(missatge);

    navegant->fd_connexio = fd_nou;
    navegant->estat = ESTAT_DESCONNECTAT;

    return arribarIlla(navegant);
}

int executarComanda (Navegant *navegant, char *linea) {
    char **trossos = NULL;
    int n_trossos;
    int codi;
    int final = FINAL_NO;

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
    } else if (codi == CMD_SAIL) {
        final = ferSail(navegant, trossos[1]);
    } else if (codi != CMD_ERROR_SINTAXI) {
        printF(1, MSG_COMANDA_OK);
    }

    freeTokens(trossos, n_trossos);

    return final;
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
