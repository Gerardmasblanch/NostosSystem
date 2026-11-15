/***********************************************
*
* @Proposit: Implementacio de la travessia de l'Odysseus, bucle principal i
*            comandes del navegant.
* @Autor/s: Arnau Ricart i Gerard Mas
* @Data creacio: 29/10/2026
* @Data ultima modificacio: 15/11/2026
*
************************************************/

//Include del fitxer .h del mateix modul
#include "travessia.h"

//Procediments i funcions

int TRAVESSIA_prepararSetOdysseus (fd_set *set, Navegant *navegant) {
    int max_fd = 0;

    FD_ZERO(set);

    FD_SET(0, set);

    if (navegant->fd_connexio != ERR_CODI) {
        FD_SET(navegant->fd_connexio, set);
        max_fd = navegant->fd_connexio;
    }

    return max_fd;
}

void TRAVESSIA_perdreConnexio (Navegant *navegant) {
    char *missatge = NULL;

    asprintf(&missatge, MSG_CONNEXIO_PERDUDA, navegant->ubicacio);
    UTILS_printF(1, missatge);
    free(missatge);

    alarm(0);
    close(navegant->fd_connexio);
    navegant->fd_connexio = ERR_CODI;

    if (strcmp(navegant->ubicacio, TEXT_ITHACA) == 0) {
        navegant->estat = ESTAT_ITHACA;
    } else {
        navegant->estat = ESTAT_DESCONNECTAT;
    }
}

void TRAVESSIA_atendreServidor (Navegant *navegant) {
    char *dades = NULL;
    int tipus;
    int flags;

    if (TRAMA_rebreTrama(navegant->fd_connexio, &tipus, &flags, &dades) == TRAMA_ERROR) {
        TRAVESSIA_perdreConnexio(navegant);
        UTILS_printF(1, TEXT_PROMPT);
    } else if (navegant->estat == ESTAT_ESPERANT && tipus == TIPUS_ARRIBADA && flags == FLAG_OK && strcmp(dades, TEXT_ATRACAT) == 0) {
        alarm(0);
        TRAVESSIA_atracar(navegant, MSG_ACCES_ESPERA);
        UTILS_printF(1, TEXT_PROMPT);
    }

    free(dades);
}

int TRAVESSIA_rebreResposta (Navegant *navegant, int tipus_esperat, char **dades) {
    int tipus;
    int flags;
    int resultat = RESPOSTA_ERROR;

    if (TRAMA_rebreTrama(navegant->fd_connexio, &tipus, &flags, dades) == TRAMA_ERROR) {
        TRAVESSIA_perdreConnexio(navegant);
        resultat = RESPOSTA_PERDUDA;
    } else if (tipus == tipus_esperat && flags == FLAG_OK) {
        resultat = RESPOSTA_OK;
    }

    return resultat;
}

void TRAVESSIA_ferConnectIthaca (Navegant *navegant) {
    char *dades = NULL;
    char *missatge = NULL;
    int resultat;

    if (navegant->estat == ESTAT_CONNECTAT_ITHACA) {
        UTILS_printF(1, MSG_JA_CONNECTAT);
        return;
    }

    if (navegant->viatge_lliurat == VIATGE_LLIURAT && (navegant->estat == ESTAT_ATRACAT || navegant->estat == ESTAT_DESCONNECTAT)) {
        if (TRAVESSIA_tornarAIthaca(navegant) == ERR_CODI) {
            return;
        }
    }

    if (navegant->estat != ESTAT_ITHACA) {
        UTILS_printF(1, MSG_NOMES_A_ITHACA);
        return;
    }

    navegant->fd_connexio = TRAMA_connectarServidor(navegant->config.ithaca_ip, navegant->config.ithaca_port);

    if (navegant->fd_connexio == ERR_CODI) {
        asprintf(&missatge, MSG_ERROR_CONNEXIO, TEXT_ITHACA);
        UTILS_printF(1, missatge);
        free(missatge);
        return;
    }

    TRAMA_enviarTrama(navegant->fd_connexio, TIPUS_CONNECT, FLAG_PETICIO, navegant->config.name);
    resultat = TRAVESSIA_rebreResposta(navegant, TIPUS_CONNECT, &dades);

    if (resultat == RESPOSTA_OK) {
        navegant->estat = ESTAT_CONNECTAT_ITHACA;

        asprintf(&missatge, MSG_CONNECTAT, TEXT_ITHACA);
        UTILS_printF(1, missatge);
        free(missatge);

    } else if (resultat == RESPOSTA_ERROR) {
        asprintf(&missatge, MSG_ERROR_RESPOSTA, dades);
        UTILS_printF(1, missatge);
        free(missatge);

        close(navegant->fd_connexio);
        navegant->fd_connexio = ERR_CODI;
    }

    free(dades);
}

int TRAVESSIA_tornarAIthaca (Navegant *navegant) {
    TRAVESSIA_marxarUbicacio(navegant);
    UTILS_printF(1, MSG_TORNANT_ITHACA);

    if (NAVEGANT_canviarUbicacio(navegant, TEXT_ITHACA) == ERR_CODI) {
        navegant->estat = ESTAT_DESCONNECTAT;
        return ERR_CODI;
    }

    navegant->estat = ESTAT_ITHACA;
    return 0;
}

int TRAVESSIA_mostrarViatge (char *dades, int *total) {
    char **trossos = NULL;
    char *missatge = NULL;
    int n_trossos;
    int resultat = TRAMA_ERROR;

    trossos = UTILS_splitString(dades, '&', &n_trossos);

    if (n_trossos == 6) {
        *total = atoi(trossos[1]);

        asprintf(&missatge, FORMAT_VIATGE, trossos[2], trossos[3], trossos[4], trossos[5]);
        UTILS_printF(1, missatge);
        free(missatge);

        resultat = TRAMA_OK;
    }

    UTILS_freeTokens(trossos, n_trossos);

    return resultat;
}

void TRAVESSIA_ferListVoyages (Navegant *navegant) {
    char *dades = NULL;
    char *missatge = NULL;
    int resultat;
    int rebudes = 0;
    int total = 1;

    if (navegant->estat != ESTAT_CONNECTAT_ITHACA) {
        UTILS_printF(1, MSG_NO_CONNECTAT_ITHACA);
        return;
    }

    if (navegant->te_viatge == AMB_VIATGE) {
        UTILS_printF(1, MSG_JA_TE_VIATGE);
        return;
    }    

    TRAMA_enviarTrama(navegant->fd_connexio, TIPUS_LIST_VOYAGES, FLAG_PETICIO, "");
    UTILS_printF(1, MSG_CAPCALERA_VIATGES);

    while (rebudes < total) {
        resultat = TRAVESSIA_rebreResposta(navegant, TIPUS_LIST_VOYAGES, &dades);

        if (resultat == RESPOSTA_PERDUDA) {
            rebudes = total;
        } else if (resultat == RESPOSTA_ERROR) {
            asprintf(&missatge, MSG_ERROR_RESPOSTA, dades);
            UTILS_printF(1, missatge);
            free(missatge);
            rebudes = total;
        } else if (strcmp(dades, TEXT_SENSE_VIATGES) == 0) {
            UTILS_printF(1, MSG_SENSE_VIATGES);
            rebudes = total;
        } else if (TRAVESSIA_mostrarViatge(dades, &total) == TRAMA_OK) {
            rebudes++;
        } else {
            UTILS_printF(1, MSG_RESPOSTA_INCORRECTA);
            rebudes = total;
        }

        free(dades);
        dades = NULL;
    }
}

int TRAVESSIA_guardarViatge (Navegant *navegant, char *dades) {
    char **trossos = NULL;
    int n_trossos;
    int resultat = TRAMA_ERROR;

    trossos = UTILS_splitString(dades, '&', &n_trossos);

    if (n_trossos == 5) {
        navegant->viatge.object = UTILS_copiarText(trossos[2]);
        navegant->viatge.destination = UTILS_copiarText(trossos[3]);

        if (navegant->viatge.object != NULL && navegant->viatge.destination != NULL) {
            navegant->viatge.id = atoi(trossos[1]);
            navegant->viatge.reward = atoi(trossos[4]);
            navegant->te_viatge = AMB_VIATGE;
            resultat = TRAMA_OK;
        } else {
            UTILS_printF(1, MSG_SENSE_MEMORIA);
            free(navegant->viatge.object);
            free(navegant->viatge.destination);
            navegant->viatge.object = NULL;
            navegant->viatge.destination = NULL;
        }
    }

    UTILS_freeTokens(trossos, n_trossos);

    return resultat;
}

void TRAVESSIA_ferAccept (Navegant *navegant, char *id_viatge) {
    char *dades = NULL;
    char *missatge = NULL;
    int resultat;

    if (navegant->estat != ESTAT_CONNECTAT_ITHACA) {
        UTILS_printF(1, MSG_NO_CONNECTAT_ITHACA);
        return;
    }

    if (navegant->te_viatge == AMB_VIATGE) {
        UTILS_printF(1, MSG_JA_TE_VIATGE);
        return;
    }

    TRAMA_enviarTrama(navegant->fd_connexio, TIPUS_ACCEPT, FLAG_PETICIO, id_viatge);
    resultat = TRAVESSIA_rebreResposta(navegant, TIPUS_ACCEPT, &dades);

    if (resultat == RESPOSTA_ERROR) {
        asprintf(&missatge, MSG_ERROR_RESPOSTA, dades);
        UTILS_printF(1, missatge);
        free(missatge);
    } else if (resultat == RESPOSTA_OK) {
        if (TRAVESSIA_guardarViatge(navegant, dades) == TRAMA_OK) {
            asprintf(&missatge, MSG_VIATGE_ACCEPTAT, navegant->viatge.id, navegant->viatge.destination);
            UTILS_printF(1, missatge);
            free(missatge);
            TRAVESSIA_rebreObjecte(navegant);
        } else {
            UTILS_printF(1, MSG_RESPOSTA_INCORRECTA);
        }
    }

    free(dades);
}

void TRAVESSIA_rebreObjecte (Navegant *navegant) {
    char *path = NULL;
    char *missatge = NULL;

    if (FITXER_rebre(navegant->fd_connexio, navegant->config.folder, TEXT_FITXER_OBJECT, "", &path) != FITXER_OK) {
        UTILS_printF(1, MSG_ERROR_OBJECTE);
        NAVEGANT_esborrarViatge(navegant);
        TRAVESSIA_perdreConnexio(navegant);
        return;
    }

    navegant->viatge.file = path;

    if (asprintf(&missatge, MSG_OBJECTE_REBUT, path + strlen(navegant->config.folder) + 1) >= 0) {
        UTILS_printF(1, missatge);
        free(missatge);
    } else {
        UTILS_printF(1, MSG_SENSE_MEMORIA);
    }

    NAVEGANT_guardarEstat(navegant);
}

int TRAVESSIA_comprovarDeliver (Navegant *navegant) {
    char *missatge = NULL;
    int resultat = TRAMA_ERROR;

    if (navegant->te_viatge != AMB_VIATGE || navegant->viatge.file == NULL) {
        UTILS_printF(1, MSG_SENSE_VIATGE_DELIVER);
    } else if (navegant->viatge_lliurat == VIATGE_LLIURAT) {
        UTILS_printF(1, MSG_JA_LLIURAT);
    } else if (navegant->estat != ESTAT_ATRACAT) {
        UTILS_printF(1, MSG_NO_ATRACAT);
    } else if (strcasecmp(navegant->ubicacio, navegant->viatge.destination) != 0) {
        if (asprintf(&missatge, MSG_NO_ES_DESTI, navegant->viatge.destination) >= 0) {
            UTILS_printF(1, missatge);
            free(missatge);
        } else {
            UTILS_printF(1, MSG_SENSE_MEMORIA);
        }
    } else {
        resultat = TRAMA_OK;
    }

    return resultat;
}

int TRAVESSIA_transferirLliurament (Navegant *navegant, char **path_segellat) {
    char *missatge = NULL;
    int resultat;

    *path_segellat = NULL;

    resultat = FITXER_enviar(navegant->fd_connexio, navegant->viatge.file, navegant->viatge.file + strlen(navegant->config.folder) + 1, TEXT_FITXER_DELIVERY, 0);
    if (resultat != FITXER_OK) {
        return resultat;
    }
    UTILS_printF(1, MSG_OBJECTE_TRANSFERIT);

    if (asprintf(&missatge, MSG_SEGELLANT, navegant->viatge.destination) >= 0) {
        UTILS_printF(1, missatge);
        free(missatge);
    } else {
        UTILS_printF(1, MSG_SENSE_MEMORIA);
    }

    resultat = FITXER_rebre(navegant->fd_connexio, navegant->config.folder, TEXT_FITXER_SIGNED, TEXT_PREFIX_SEGELLAT, path_segellat);
    if (resultat == FITXER_OK) {
        UTILS_printF(1, MSG_SEGELLAT_REBUT);
    }

    return resultat;
}

void TRAVESSIA_acabarDeliver (Navegant *navegant, int resultat_fitxer, char *path_segellat) {
    char *dades = NULL;
    char *missatge = NULL;
    int resultat;

    resultat = TRAVESSIA_rebreResposta(navegant, TIPUS_DELIVER, &dades);

    if (resultat == RESPOSTA_OK && resultat_fitxer == FITXER_OK) {
        free(navegant->fitxer_segellat);
        navegant->fitxer_segellat = path_segellat;
        navegant->viatge_lliurat = VIATGE_LLIURAT;
        NAVEGANT_guardarEstat(navegant);
        if (asprintf(&missatge, MSG_LLIURAMENT_FET, navegant->viatge.destination) >= 0) {
            UTILS_printF(1, missatge);
            free(missatge);
        } else {
            UTILS_printF(1, MSG_SENSE_MEMORIA);
        }
    } else {
        free(path_segellat);
        if (resultat == RESPOSTA_ERROR && asprintf(&missatge, MSG_ERROR_RESPOSTA, dades) >= 0) {
            UTILS_printF(1, missatge);
            free(missatge);
        } else if (resultat == RESPOSTA_OK) {
            UTILS_printF(1, MSG_ERROR_LLIURAMENT);
        }
    }

    free(dades);
}

void TRAVESSIA_ferDeliver (Navegant *navegant) {
    char *peticio = NULL;
    char *missatge = NULL;
    char *path_segellat = NULL;
    int resultat;

    if (TRAVESSIA_comprovarDeliver(navegant) == TRAMA_ERROR) {
        return;
    }

    if (asprintf(&peticio, FORMAT_PETICIO_DELIVER, navegant->viatge.id, navegant->viatge.object) < 0) {
        UTILS_printF(1, MSG_SENSE_MEMORIA);
        return;
    }

    if (asprintf(&missatge, MSG_LLIURANT, navegant->viatge.object, navegant->viatge.destination) >= 0) {
        UTILS_printF(1, missatge);
        free(missatge);
    } else {
        UTILS_printF(1, MSG_SENSE_MEMORIA);
    }

    resultat = TRAMA_enviarTrama(navegant->fd_connexio, TIPUS_DELIVER, FLAG_PETICIO, peticio);
    free(peticio);
    if (resultat == TRAMA_ERROR) {
        TRAVESSIA_perdreConnexio(navegant);
        return;
    }

    resultat = TRAVESSIA_transferirLliurament(navegant, &path_segellat);
    if (resultat == FITXER_ERROR) {
        UTILS_printF(1, MSG_ERROR_LLIURAMENT);
        TRAVESSIA_perdreConnexio(navegant);
        return;
    }

    TRAVESSIA_acabarDeliver(navegant, resultat, path_segellat);
}

int TRAVESSIA_comprovarClaim (Navegant *navegant) {
    int resultat = TRAMA_ERROR;

    if (navegant->estat != ESTAT_CONNECTAT_ITHACA) {
        UTILS_printF(1, MSG_NO_CONNECTAT_ITHACA);
    } else if (navegant->te_viatge != AMB_VIATGE || navegant->viatge_lliurat != VIATGE_LLIURAT || navegant->fitxer_segellat == NULL) {
        UTILS_printF(1, MSG_SENSE_LLIURAMENT);
    } else {
        resultat = TRAMA_OK;
    }

    return resultat;
}

void TRAVESSIA_cobrarRecompensa (Navegant *navegant, char *dades) {
    char **trossos = NULL;
    char *missatge = NULL;
    int n_trossos = 0;
    int recompensa;

    trossos = UTILS_splitString(dades, '&', &n_trossos);

    if (n_trossos != N_CAMPS_CLAIM || strcmp(trossos[0], TEXT_CLAIM_OK) != 0 || UTILS_esNumeric(trossos[1]) == TEXT_NO_NUMERIC) {
        UTILS_freeTokens(trossos, n_trossos);
        UTILS_printF(1, MSG_RESPOSTA_INCORRECTA);
        return;
    }

    recompensa = atoi(trossos[1]);
    UTILS_freeTokens(trossos, n_trossos);
    navegant->config.gold = navegant->config.gold + recompensa;

    if (asprintf(&missatge, MSG_CLAIM_FET, navegant->viatge.destination, navegant->viatge.id, recompensa) >= 0) {
        UTILS_printF(1, missatge);
        free(missatge);
    } else {
        UTILS_printF(1, MSG_SENSE_MEMORIA);
    }

    NAVEGANT_esborrarViatge(navegant);
    NAVEGANT_guardarEstat(navegant);
}

void TRAVESSIA_ferClaim (Navegant *navegant) {
    char *peticio = NULL;
    char *dades = NULL;
    char *missatge = NULL;
    int resultat;
    int resultat_fitxer;

    if (TRAVESSIA_comprovarClaim(navegant) == TRAMA_ERROR) {
        return;
    }

    if (asprintf(&peticio, FORMAT_PETICIO_CLAIM, navegant->viatge.id) < 0) {
        UTILS_printF(1, MSG_SENSE_MEMORIA);
        return;
    }

    if (asprintf(&missatge, MSG_PRESENTANT, navegant->viatge.object) >= 0) {
        UTILS_printF(1, missatge);
        free(missatge);
    } else {
        UTILS_printF(1, MSG_SENSE_MEMORIA);
    }

    resultat = TRAMA_enviarTrama(navegant->fd_connexio, TIPUS_CLAIM, FLAG_PETICIO, peticio);
    free(peticio);
    if (resultat == TRAMA_ERROR) {
        TRAVESSIA_perdreConnexio(navegant);
        return;
    }

    resultat_fitxer = FITXER_enviar(navegant->fd_connexio, navegant->fitxer_segellat, navegant->viatge.file + strlen(navegant->config.folder) + 1, TEXT_FITXER_CLAIM, 0);
    if (resultat_fitxer == FITXER_ERROR) {
        UTILS_printF(1, MSG_ERROR_CLAIM);
        TRAVESSIA_perdreConnexio(navegant);
        return;
    }
    if (resultat_fitxer == FITXER_OK) {
        UTILS_printF(1, MSG_OBJECTE_TRANSFERIT);
    }

    resultat = TRAVESSIA_rebreResposta(navegant, TIPUS_CLAIM, &dades);
    if (resultat == RESPOSTA_OK && resultat_fitxer == FITXER_OK) {
        TRAVESSIA_cobrarRecompensa(navegant, dades);
    } else if (resultat == RESPOSTA_ERROR && asprintf(&missatge, MSG_ERROR_RESPOSTA, dades) >= 0) {
        UTILS_printF(1, missatge);
        free(missatge);
    } else if (resultat == RESPOSTA_OK) {
        UTILS_printF(1, MSG_ERROR_CLAIM);
    }

    free(dades);
}

void TRAVESSIA_marxarUbicacio (Navegant *navegant) {
    char *dades = NULL;
    char *missatge = NULL;

    asprintf(&missatge, MSG_MARXANT, navegant->ubicacio);
    UTILS_printF(1, missatge);
    free(missatge);

    if (navegant->estat == ESTAT_CONNECTAT_ITHACA) {
        TRAMA_enviarTrama(navegant->fd_connexio, TIPUS_SORTIR_ITHACA, FLAG_PETICIO, "");
        TRAVESSIA_rebreResposta(navegant, TIPUS_SORTIR_ITHACA, &dades);
    } else if (navegant->estat == ESTAT_ATRACAT) {
        TRAMA_enviarTrama(navegant->fd_connexio, TIPUS_SORTIR_ILLA, FLAG_PETICIO, "");
        TRAVESSIA_rebreResposta(navegant, TIPUS_SORTIR_ILLA, &dades);
    }

    free(dades);

    if (navegant->fd_connexio != ERR_CODI) {
        close(navegant->fd_connexio);
        navegant->fd_connexio = ERR_CODI;
    }
}

void TRAVESSIA_atracar (Navegant *navegant, char *format) {
    char *missatge = NULL;

    navegant->estat = ESTAT_ATRACAT;
    navegant->mercat_consultat = 0;

    asprintf(&missatge, format, navegant->ubicacio);
    UTILS_printF(1, missatge);
    free(missatge);

    if (navegant->te_viatge == AMB_VIATGE && strcasecmp(navegant->ubicacio, navegant->viatge.destination) == 0) {
        asprintf(&missatge, MSG_DESTI, navegant->ubicacio);
        UTILS_printF(1, missatge);
        free(missatge);
    }
}

int TRAVESSIA_gestionarRacio (Navegant *navegant) {
    char *missatge = NULL;
    int final = FINAL_NO;

    if (NAVEGANT_totalProvisions(navegant) <= RACIO_KG) {
        asprintf(&missatge, MSG_SENSE_PROVISIONS, navegant->config.name);
        UTILS_printF(1, missatge);
        free(missatge);
        NAVEGANT_esborrarViatge(navegant);
        navegant->estat = ESTAT_PERDUT;
        final = FINAL_SI;
    } else {
        NAVEGANT_consumirRacio(navegant, RACIO_KG);
        asprintf(&missatge, MSG_RACIO, NAVEGANT_totalProvisions(navegant));
        UTILS_printF(1, missatge);
        free(missatge);
        UTILS_printF(1, TEXT_PROMPT);
        alarm(SEGONS_RACIO);
    }

    return final;
}

void TRAVESSIA_arribarIlla (Navegant *navegant) {
    char *dades = NULL;
    char *missatge = NULL;
    int resultat;

    UTILS_printF(1, "\n");
    asprintf(&missatge, MSG_CONNECTAT, navegant->ubicacio);
    UTILS_printF(1, missatge);
    free(missatge);

    TRAMA_enviarTrama(navegant->fd_connexio, TIPUS_ARRIBADA, FLAG_PETICIO, navegant->config.name);
    resultat = TRAVESSIA_rebreResposta(navegant, TIPUS_ARRIBADA, &dades);

    if (resultat == RESPOSTA_OK && strcmp(dades, TEXT_ATRACAT) == 0) {
        TRAVESSIA_atracar(navegant, MSG_ACCES);
    } else if (resultat == RESPOSTA_OK && strcmp(dades, TEXT_ESPERA) == 0) {
        UTILS_printF(1, MSG_PORT_COMPLET);
        navegant->estat = ESTAT_ESPERANT;
        alarm(SEGONS_RACIO);
    } else if (resultat != RESPOSTA_PERDUDA) {
        if (resultat == RESPOSTA_ERROR) {
            asprintf(&missatge, MSG_ERROR_RESPOSTA, dades);
            UTILS_printF(1, missatge);
            free(missatge);
        } else {
            UTILS_printF(1, MSG_RESPOSTA_INCORRECTA);
        }
        close(navegant->fd_connexio);
        navegant->fd_connexio = ERR_CODI;
    }

    free(dades);
}

void TRAVESSIA_tornarAIlla (Navegant *navegant) {
    char *missatge = NULL;
    char *ip = NULL;
    int port = 0;
    int index;

    if (strcasecmp(navegant->ubicacio, TEXT_AEAEA) == 0) {
        ip = navegant->config.aeaea_ip;
        port = navegant->config.aeaea_port;
    } else {
        index = NAVEGANT_buscarAdreca(navegant, navegant->ubicacio);
        if (index != ERR_CODI) {
            ip = navegant->adreces[index].ip;
            port = navegant->adreces[index].port;
        }
    }

    if (ip != NULL) {
        navegant->fd_connexio = TRAMA_connectarServidor(ip, port);
    }

    if (navegant->fd_connexio == ERR_CODI) {
        asprintf(&missatge, MSG_ERROR_CONNEXIO, navegant->ubicacio);
        UTILS_printF(1, missatge);
        free(missatge);
        return;
    }

    TRAVESSIA_arribarIlla(navegant);
}

void TRAVESSIA_ferSail (Navegant *navegant, char *illa) {
    char *missatge = NULL;
    char *nom = NULL;
    char *ip = NULL;
    int port = 0;
    int index;
    int fd_nou;

    if (navegant->estat == ESTAT_ESPERANT) {
        UTILS_printF(1, MSG_ESPERANT_PORT);
        return;
    }

    if (navegant->estat == ESTAT_DESCONNECTAT && strcasecmp(illa, navegant->ubicacio) == 0) {
        TRAVESSIA_tornarAIlla(navegant);
        return;
    }

    if (navegant->estat == ESTAT_ITHACA) {
        UTILS_printF(1, MSG_NO_CONNECTAT_ITHACA);
        return;
    }

    if (navegant->estat == ESTAT_CONNECTAT_ITHACA) {
        if (navegant->te_viatge == SENSE_VIATGE) {
            UTILS_printF(1, MSG_SENSE_VIATGE_SAIL);
            return;
        }
        if (strcasecmp(illa, TEXT_AEAEA) == 0) {
            nom = TEXT_AEAEA;
            ip = navegant->config.aeaea_ip;
            port = navegant->config.aeaea_port;
        }
    } else if (NAVEGANT_esRutaDirecta(navegant, illa) == RUTA_DIRECTA) {
        index = NAVEGANT_buscarAdreca(navegant, illa);
        if (index != ERR_CODI) {
            nom = navegant->adreces[index].name;
            ip = navegant->adreces[index].ip;
            port = navegant->adreces[index].port;
        }
    }

    if (nom == NULL) {
        asprintf(&missatge, MSG_NO_ES_POT_NAVEGAR, illa);
        UTILS_printF(1, missatge);
        free(missatge);
        return;
    }

    fd_nou = TRAMA_connectarServidor(ip, port);
    if (fd_nou == ERR_CODI) {
        asprintf(&missatge, MSG_ERROR_CONNEXIO, nom);
        UTILS_printF(1, missatge);
        free(missatge);
        return;
    }

    TRAVESSIA_marxarUbicacio(navegant);

    if (NAVEGANT_canviarUbicacio(navegant, nom) == ERR_CODI) {
        close(fd_nou);
        navegant->estat = ESTAT_DESCONNECTAT;
        return;
    }

    asprintf(&missatge, MSG_NAVEGANT, nom);
    UTILS_printF(1, missatge);
    free(missatge);

    navegant->fd_connexio = fd_nou;
    navegant->estat = ESTAT_DESCONNECTAT;

    TRAVESSIA_arribarIlla(navegant);
}

void TRAVESSIA_mostrarCapcaleraMercat (char *illa) {
    char *majuscules = NULL;
    char *missatge = NULL;
    int i;

    majuscules = UTILS_copiarText(illa);
    if (majuscules == NULL) {
        UTILS_printF(1, MSG_SENSE_MEMORIA);
        return;
    }

    for (i = 0; majuscules[i] != '\0'; i++) {
        if (majuscules[i] >= 'a' && majuscules[i] <= 'z') {
            majuscules[i] = majuscules[i] - 'a' + 'A';
        }
    }

    asprintf(&missatge, MSG_CAPCALERA_MERCAT, majuscules);
    UTILS_printF(1, missatge);
    free(missatge);
    free(majuscules);
}

void TRAVESSIA_mostrarProducte (char *nom, char *quantitat, char *preu) {
    char *missatge = NULL;

    if (strcasecmp(nom, TEXT_MAPA) == 0) {
        asprintf(&missatge, FORMAT_MAPA_MERCAT, TEXT_MAPA_MERCAT, preu);
    } else {
        asprintf(&missatge, FORMAT_PRODUCTE_MERCAT, nom, quantitat, preu);
    }

    UTILS_printF(1, missatge);
    free(missatge);
}

int TRAVESSIA_rebreMercat (Navegant *navegant, int mostrar, char *producte, int *preu) {
    char **trossos = NULL;
    char *dades = NULL;
    char *missatge = NULL;
    int n_trossos;
    int resultat = RESPOSTA_OK;
    int rebudes = 0;
    int total = 1;

    TRAMA_enviarTrama(navegant->fd_connexio, TIPUS_LIST_MARKET, FLAG_PETICIO, "");

    while (rebudes < total) {
        resultat = TRAVESSIA_rebreResposta(navegant, TIPUS_LIST_MARKET, &dades);

        if (resultat == RESPOSTA_ERROR) {
            asprintf(&missatge, MSG_ERROR_RESPOSTA, dades);
            UTILS_printF(1, missatge);
            free(missatge);
            rebudes = total;
        } else if (resultat == RESPOSTA_PERDUDA) {
            rebudes = total;
        } else {
            trossos = UTILS_splitString(dades, '&', &n_trossos);

            if (n_trossos == 5) {
                total = atoi(trossos[1]);

                if (mostrar == MOSTRAR_SI) {
                    if (rebudes == 0) {
                        TRAVESSIA_mostrarCapcaleraMercat(navegant->ubicacio);
                    }
                    TRAVESSIA_mostrarProducte(trossos[2], trossos[3], trossos[4]);
                }

                if (producte != NULL && strcasecmp(trossos[2], producte) == 0) {
                    *preu = atoi(trossos[4]);
                }

                rebudes++;
            } else {
                UTILS_printF(1, MSG_RESPOSTA_INCORRECTA);
                resultat = RESPOSTA_ERROR;
                rebudes = total;
            }

            UTILS_freeTokens(trossos, n_trossos);
        }

        free(dades);
        dades = NULL;
    }

    return resultat;
}

void TRAVESSIA_ferListMarket (Navegant *navegant) {
    if (navegant->estat != ESTAT_ATRACAT) {
        UTILS_printF(1, MSG_NO_ATRACAT);
        return;
    }

    if (TRAVESSIA_rebreMercat(navegant, MOSTRAR_SI, NULL, NULL) == RESPOSTA_OK) {
        navegant->mercat_consultat = 1;
    }
}

void TRAVESSIA_ferBuy (Navegant *navegant, char *producte, char *text_quantitat) {
    char *peticio = NULL, *dades = NULL, *missatge = NULL;
    int quantitat = atoi(text_quantitat);
    int preu = -1;
    int index, cost, resultat;

    if (navegant->estat != ESTAT_ATRACAT) {
        UTILS_printF(1, MSG_NO_ATRACAT);
        return;
    }
    if (navegant->mercat_consultat == 0) {
        UTILS_printF(1, MSG_CAL_LIST_MARKET);
        return;
    }
    if (quantitat <= 0) {
        UTILS_printF(1, MSG_QUANTITAT_INVALIDA);
        return;
    }
    if (TRAVESSIA_rebreMercat(navegant, MOSTRAR_NO, producte, &preu) != RESPOSTA_OK) {
        return;
    }
    if (preu < 0) {
        UTILS_printF(1, MSG_PRODUCTE_DESCONEGUT);
        return;
    }
    if (preu > 0 && quantitat > navegant->config.gold / preu) {
        UTILS_printF(1, MSG_OR_INSUFICIENT);
        return;
    }
    index = NAVEGANT_prepararAliment(navegant, producte);
    if (index == ERR_CODI) {
        return;
    }
    if (asprintf(&peticio, FORMAT_PETICIO_COMPRA, producte, quantitat) < 0) {
        UTILS_printF(1, MSG_SENSE_MEMORIA);
        return;
    }
    resultat = TRAMA_enviarTrama(navegant->fd_connexio, TIPUS_BUY, FLAG_PETICIO, peticio);
    free(peticio);
    if (resultat == TRAMA_ERROR) {
        TRAVESSIA_perdreConnexio(navegant);
        return;
    }
    resultat = TRAVESSIA_rebreResposta(navegant, TIPUS_BUY, &dades);
    if (resultat == RESPOSTA_OK) {
        if (NAVEGANT_guardarCompra(navegant, index, quantitat, dades, &cost) == TRAMA_OK) {
            if (asprintf(&missatge, MSG_COMPRA_FETA, quantitat, navegant->config.foods[index].name, cost) >= 0) {
                UTILS_printF(1, missatge);
                free(missatge);
            } else {
                UTILS_printF(1, MSG_SENSE_MEMORIA);
            }
        } else {
            UTILS_printF(1, MSG_RESPOSTA_INCORRECTA);
        }
    } else if (resultat == RESPOSTA_ERROR) {
        if (asprintf(&missatge, MSG_ERROR_RESPOSTA, dades) >= 0) {
            UTILS_printF(1, missatge);
            free(missatge);
        } else {
            UTILS_printF(1, MSG_SENSE_MEMORIA);
        }
    }
    free(dades);
}

void TRAVESSIA_ferSell (Navegant *navegant, char *producte, char *text_quantitat) {
    char *peticio = NULL, *dades = NULL, *missatge = NULL;
    int quantitat = atoi(text_quantitat);
    int preu = -1;
    int index, ingressos, resultat;

    if (navegant->estat != ESTAT_ATRACAT) {
        UTILS_printF(1, MSG_NO_ATRACAT);
        return;
    }
    if (quantitat <= 0) {
        UTILS_printF(1, MSG_QUANTITAT_INVALIDA);
        return;
    }

    index = NAVEGANT_buscarAliment(navegant, producte);
    if (index == ERR_CODI) {
        if (TRAVESSIA_rebreMercat(navegant, MOSTRAR_NO, producte, &preu) != RESPOSTA_OK) {
            return;
        }
        if (preu < 0) {
            UTILS_printF(1, MSG_PRODUCTE_DESCONEGUT);
        } else {
            asprintf(&missatge, MSG_NO_EL_PORTES, producte);
            UTILS_printF(1, missatge);
            free(missatge);
        }
        return;
    }
    if (navegant->config.foods[index].amount == 0) {
        asprintf(&missatge, MSG_NO_EL_PORTES, navegant->config.foods[index].name);
        UTILS_printF(1, missatge);
        free(missatge);
        return;
    }
    if (quantitat > navegant->config.foods[index].amount) {
        UTILS_printF(1, MSG_CARREGA_INSUFICIENT);
        return;
    }

    if (asprintf(&peticio, FORMAT_PETICIO_VENDA, producte, quantitat) < 0) {
        UTILS_printF(1, MSG_SENSE_MEMORIA);
        return;
    }
    resultat = TRAMA_enviarTrama(navegant->fd_connexio, TIPUS_SELL, FLAG_PETICIO, peticio);
    free(peticio);
    if (resultat == TRAMA_ERROR) {
        TRAVESSIA_perdreConnexio(navegant);
        return;
    }

    resultat = TRAVESSIA_rebreResposta(navegant, TIPUS_SELL, &dades);
    if (resultat == RESPOSTA_OK) {
        if (NAVEGANT_guardarVenda(navegant, index, quantitat, dades, &ingressos) == TRAMA_OK) {
            if (asprintf(&missatge, MSG_VENDA_FETA, quantitat, navegant->config.foods[index].name, ingressos) >= 0) {
                UTILS_printF(1, missatge);
                free(missatge);
            } else {
                UTILS_printF(1, MSG_SENSE_MEMORIA);
            }
        } else {
            UTILS_printF(1, MSG_RESPOSTA_INCORRECTA);
        }
    } else if (resultat == RESPOSTA_ERROR) {
        if (asprintf(&missatge, MSG_ERROR_RESPOSTA, dades) >= 0) {
            UTILS_printF(1, missatge);
            free(missatge);
        } else {
            UTILS_printF(1, MSG_SENSE_MEMORIA);
        }
    }
    free(dades);
}

void TRAVESSIA_executarComanda (Navegant *navegant, char *linea) {
    char **trossos = NULL;
    int n_trossos;
    int codi;

    codi = COMMANDS_parseCommand(linea);
    trossos = UTILS_splitString(linea, ' ', &n_trossos);

    if (codi == CMD_DESCONEGUDA) {
        UTILS_printF(1, MSG_DESCONEGUDA);
    } else if (codi == CMD_CONNECT_ITHACA) {
        TRAVESSIA_ferConnectIthaca(navegant);
    } else if (codi == CMD_LIST_VOYAGES) {
        TRAVESSIA_ferListVoyages(navegant);
    } else if (codi == CMD_ACCEPT) {
        TRAVESSIA_ferAccept(navegant, trossos[1]);
    } else if (codi == CMD_SAIL) {
        TRAVESSIA_ferSail(navegant, trossos[1]);
    } else if (codi == CMD_LIST_MARKET) {
        TRAVESSIA_ferListMarket(navegant);
    } else if (codi == CMD_BUY) {
        if (strcasecmp(trossos[1], TEXT_MAPA) == 0) {
            TRAVESSIA_ferBuyMap(navegant, trossos[2]);
        } else {
            TRAVESSIA_ferBuy(navegant, trossos[1], trossos[2]);
        }
    } else if (codi == CMD_SELL) {
        TRAVESSIA_ferSell(navegant, trossos[1], trossos[2]);
    } else if (codi == CMD_STATUS) {
        NAVEGANT_mostrarStatus(navegant);
    } else if (codi == CMD_MAP) {
        TRAVESSIA_ferMap(navegant);
    } else if (codi == CMD_DELIVER) {
        TRAVESSIA_ferDeliver(navegant);
    } else if (codi == CMD_CLAIM) {
        TRAVESSIA_ferClaim(navegant);
    } else if (codi != CMD_ERROR_SINTAXI) {
        UTILS_printF(1, MSG_COMANDA_OK);
    }

    UTILS_freeTokens(trossos, n_trossos);
}

int TRAVESSIA_atendreTeclat (Navegant *navegant) {
    char *linea = NULL;
    int final = FINAL_SI;

    linea = UTILS_readUntil(0, '\n');

    if (linea != NULL) {
        TRAVESSIA_executarComanda(navegant, linea);
        free(linea);
        final = FINAL_NO;

        UTILS_printF(1, "\n");
        UTILS_printF(1, TEXT_PROMPT);
    }

    return final;
}

void TRAVESSIA_executarOdysseus (Navegant *navegant, int *sortir, int *racio_pendent) {
    fd_set set;
    int max_fd;
    int resultat;
    int final = FINAL_NO;

    UTILS_printF(1, TEXT_PROMPT);

    while (*sortir == 0 && final == FINAL_NO) {
        max_fd = TRAVESSIA_prepararSetOdysseus(&set, navegant);

        resultat = 0;
        if (*racio_pendent == 0) {
            resultat = select(max_fd + 1, &set, NULL, NULL, NULL);
        }

        if (*racio_pendent == 1) {
            *racio_pendent = 0;
            if (navegant->estat == ESTAT_ESPERANT) {
                final = TRAVESSIA_gestionarRacio(navegant);
            }
        } else if (resultat > 0) {
            if (navegant->fd_connexio != ERR_CODI && FD_ISSET(navegant->fd_connexio, &set)) {
                TRAVESSIA_atendreServidor(navegant);
            }
            if (FD_ISSET(0, &set)) {
                final = TRAVESSIA_atendreTeclat(navegant);
            }
        }
    }
}

void TRAVESSIA_ferBuyMap (Navegant *navegant, char *quantitat) {
    char **trossos = NULL;
    char *dades = NULL;
    char *missatge = NULL;
    int n_trossos;
    int resultat;
    int rebudes = 0;
    int total = 1;
    int cost = 0;

    if (navegant->estat != ESTAT_ATRACAT) {
        UTILS_printF(1, MSG_NO_ATRACAT);
        return;
    }

    if (atoi(quantitat) != MAPA_QUANTITAT) {
        UTILS_printF(1, MSG_NOMES_UN_MAPA);
        return;
    }

    if (navegant->config.gold < MAPA_PREU) {
        asprintf(&missatge, MSG_OR_INSUFICIENT_MAPA, MAPA_PREU, navegant->config.gold);
        UTILS_printF(1, missatge);
        free(missatge);
        return;
    }

    TRAMA_enviarTrama(navegant->fd_connexio, TIPUS_BUY_MAP, FLAG_PETICIO, quantitat);

    while (rebudes < total) {
        resultat = TRAVESSIA_rebreResposta(navegant, TIPUS_BUY_MAP, &dades);

        if (resultat == RESPOSTA_ERROR) {
            asprintf(&missatge, MSG_ERROR_RESPOSTA, dades);
            UTILS_printF(1, missatge);
            free(missatge);
            rebudes = total;
        } else if (resultat == RESPOSTA_PERDUDA) {
            rebudes = total;
        } else {
            trossos = UTILS_splitString(dades, '&', &n_trossos);

            if (n_trossos == 3) {
                cost = atoi(trossos[1]);
                total = 0;
            } else if (n_trossos == 7) {
                cost = atoi(trossos[1]);
                total = atoi(trossos[3]);
                NAVEGANT_afegirRuta(navegant, navegant->ubicacio, trossos[4]);
                NAVEGANT_afegirAdreca(navegant, trossos[4], trossos[5], atoi(trossos[6]));
                rebudes++;
            } else {
                UTILS_printF(1, MSG_RESPOSTA_INCORRECTA);
                rebudes = total;
            }

            UTILS_freeTokens(trossos, n_trossos);
        }

        free(dades);
        dades = NULL;
    }

    if (cost > 0) {
        navegant->config.gold = navegant->config.gold - cost;

        asprintf(&missatge, MSG_MAPA_COMPRAT, navegant->ubicacio, cost);
        UTILS_printF(1, missatge);
        free(missatge);

        if (total > 0) {
            UTILS_printF(1, MSG_RUTES_NOVES);
        } else {
            UTILS_printF(1, MSG_SENSE_RUTES);
        }
    }
}

void TRAVESSIA_ferMap (Navegant *navegant) {
    int resultat;

    resultat = SPHRAGIS_display_map(navegant->mapa, navegant->n_illes);

    if (resultat == SPHRAGIS_ERROR) {
        UTILS_printF(1, MSG_ERROR_MAPA_TERMINAL);
    } else if (resultat != SPHRAGIS_OK) {
        UTILS_printF(1, MSG_ERROR_MAPA);
    }
}
