/***********************************************
*
* @Proposit: Implementacio de la gestio dels clients i dels viatges del servidor Ithaca.
* @Autor/s: Arnau Ricart i Gerard Mas
* @Data creacio: 29/10/2026
* @Data ultima modificacio: 29/10/2026
*
************************************************/

#include "voyages.h"


void VOYAGES_inicialitzarServidorIthaca (ServidorIthaca *servidor) {
    servidor->clients = NULL;
    servidor->n_clients = 0;
    servidor->viatges = NULL;
    servidor->n_viatges = 0;
}


int VOYAGES_prepararViatgesIthaca (ServidorIthaca *servidor, Voyage *viatges, int n_viatges) {
    int i;

    if (n_viatges == 0) {
        return 0;
    }

    servidor->viatges = malloc(sizeof(ViatgeIthaca) * n_viatges);

    if (servidor->viatges == NULL) {
        UTILS_printF(1, MSG_SENSE_MEMORIA);
        return ERR_CODI;
    }

    for (i = 0; i < n_viatges; i++) {
        servidor->viatges[i].viatge = &viatges[i];
        servidor->viatges[i].estat = VIATGE_DISPONIBLE;
        servidor->viatges[i].nom_odysseus = NULL;
    }

    servidor->n_viatges = n_viatges;

    return 0;
}


int VOYAGES_buscarViatgeAssignat (ServidorIthaca *servidor, char *nom) {
    int i;
    int index = ERR_CODI;

    if (nom == NULL) {
        return ERR_CODI;
    }

    for (i = 0; i < servidor->n_viatges; i++) {
        if (servidor->viatges[i].estat == VIATGE_ASSIGNAT && servidor->viatges[i].nom_odysseus != NULL) {
            if (strcmp(servidor->viatges[i].nom_odysseus, nom) == 0) {
                index = i;
            }
        }
    }

    return index;
}


int VOYAGES_buscarViatgePerId (ServidorIthaca *servidor, int id_viatge) {
    int i;
    int index = ERR_CODI;

    for (i = 0; i < servidor->n_viatges; i++) {
        if (servidor->viatges[i].viatge->id == id_viatge) {
            index = i;
        }
    }

    return index;
}


int VOYAGES_comptarViatgesDisponibles (ServidorIthaca *servidor) {
    int i;
    int disponibles = 0;

    for (i = 0; i < servidor->n_viatges; i++) {
        if (servidor->viatges[i].estat == VIATGE_DISPONIBLE) {
            disponibles++;
        }
    }

    return disponibles;
}


int VOYAGES_guardarViatgesConsultats (ServidorIthaca *servidor, int index) {
    ClientIthaca *client = NULL;
    int total;
    int i;

    client = &servidor->clients[index];

    free(client->viatges_consultats);
    client->viatges_consultats = NULL;
    client->n_viatges_consultats = 0;

    total = VOYAGES_comptarViatgesDisponibles(servidor);

    if (total == 0) {
        return 0;
    }

    client->viatges_consultats = malloc(sizeof(int) * total);

    if (client->viatges_consultats == NULL) {
        UTILS_printF(1, MSG_SENSE_MEMORIA);
        return ERR_CODI;
    }

    for (i = 0; i < servidor->n_viatges; i++) {
        if (servidor->viatges[i].estat == VIATGE_DISPONIBLE) {
            client->viatges_consultats[client->n_viatges_consultats] = servidor->viatges[i].viatge->id;
            client->n_viatges_consultats++;
        }
    }

    return 0;
}


int VOYAGES_haConsultatViatge (ClientIthaca *client, int id_viatge) {
    int i;
    int consultat = 0;

    for (i = 0; i < client->n_viatges_consultats; i++) {
        if (client->viatges_consultats[i] == id_viatge) {
            consultat = 1;
        }
    }

    return consultat;
}


int VOYAGES_enviarViatgesDisponibles (ServidorIthaca *servidor, int fd) {
    Voyage *viatge = NULL;
    char *text = NULL;
    int total;
    int numero = 0;
    int resultat;
    int i;

    total = VOYAGES_comptarViatgesDisponibles(servidor);

    if (total == 0) {
        return TRAMA_enviarTrama(fd, TIPUS_LIST_VOYAGES, FLAG_OK, TEXT_ITHACA_SENSE_VIATGES);
    }

    for (i = 0; i < servidor->n_viatges; i++) {
        if (servidor->viatges[i].estat == VIATGE_DISPONIBLE) {
            viatge = servidor->viatges[i].viatge;
            numero++;

            if (asprintf(&text, FORMAT_VIATGE_DISPONIBLE, numero, total, viatge->id, viatge->object, viatge->destination, viatge->reward) < 0) {
                UTILS_printF(1, MSG_SENSE_MEMORIA);
                return TRAMA_enviarTrama(fd, TIPUS_LIST_VOYAGES, FLAG_ERROR, TEXT_ITHACA_ERROR_SERVIDOR);
            }

            resultat = TRAMA_enviarTrama(fd, TIPUS_LIST_VOYAGES, FLAG_OK, text);
            free(text);

            if (resultat == TRAMA_ERROR) {
                return TRAMA_ERROR;
            }
        }
    }

    return TRAMA_OK;
}


int VOYAGES_atendreListVoyages (ServidorIthaca *servidor, int index, char *dades) {
    ClientIthaca *client = NULL;
    char *missatge = NULL;
    int index_viatge;

    client = &servidor->clients[index];

    if (client->nom == NULL || client->viatge_acceptat == 1) {
        return TRAMA_enviarTrama(client->fd, TIPUS_LIST_VOYAGES, FLAG_ERROR, TEXT_ITHACA_ERROR_ESTAT);
    }

    if (strlen(dades) != 0) {
        return TRAMA_enviarTrama(client->fd, TIPUS_LIST_VOYAGES, FLAG_ERROR, TEXT_ITHACA_ERROR_DADES);
    }

    index_viatge = VOYAGES_buscarViatgeAssignat(servidor, client->nom);

    if (index_viatge != ERR_CODI) {
        servidor->viatges[index_viatge].estat = VIATGE_FRACASSAT;

        if (asprintf(&missatge, MSG_VIATGE_FRACASSAT, servidor->viatges[index_viatge].viatge->id, client->nom) >= 0) {
            UTILS_printF(1, missatge);
            free(missatge);
            missatge = NULL;
        } else {
            UTILS_printF(1, MSG_SENSE_MEMORIA);
        }
    }

    if (VOYAGES_guardarViatgesConsultats(servidor, index) == ERR_CODI) {
        return TRAMA_enviarTrama(client->fd, TIPUS_LIST_VOYAGES, FLAG_ERROR, TEXT_ITHACA_ERROR_SERVIDOR);
    }

    if (asprintf(&missatge, MSG_CONSULTA_VIATGES, client->nom) >= 0) {
        UTILS_printF(1, missatge);
        free(missatge);
    } else {
        UTILS_printF(1, MSG_SENSE_MEMORIA);
    }

    return VOYAGES_enviarViatgesDisponibles(servidor, client->fd);
}


int VOYAGES_atendreAcceptIthaca (ServidorIthaca *servidor, int index, char *dades) {
    ClientIthaca *client = NULL;
    ViatgeIthaca *assignacio = NULL;
    Voyage *viatge = NULL;
    char *text = NULL;
    char *missatge = NULL;
    
    int index_viatge;
    int resultat;


    client = &servidor->clients[index];

    if (client->nom == NULL || client->viatge_acceptat == 1) {
        return TRAMA_enviarTrama(client->fd, TIPUS_ACCEPT, FLAG_ERROR, TEXT_ITHACA_ERROR_ESTAT);
    }
    if (VOYAGES_buscarViatgeAssignat(servidor, client->nom) != ERR_CODI) {
        return TRAMA_enviarTrama(client->fd, TIPUS_ACCEPT, FLAG_ERROR, TEXT_ITHACA_ERROR_ESTAT);
    }
    if (UTILS_esNumeric(dades) == TEXT_NO_NUMERIC) {
        return TRAMA_enviarTrama(client->fd, TIPUS_ACCEPT, FLAG_ERROR, TEXT_ITHACA_ERROR_DADES);
    }

    index_viatge = VOYAGES_buscarViatgePerId(servidor, atoi(dades));
    if (index_viatge == ERR_CODI ||
        VOYAGES_haConsultatViatge(client, atoi(dades)) == 0) {
        return TRAMA_enviarTrama(client->fd, TIPUS_ACCEPT, FLAG_ERROR, TEXT_ITHACA_ERROR_DADES);
    }

    assignacio = &servidor->viatges[index_viatge];
    if (assignacio->estat != VIATGE_DISPONIBLE) {
        return TRAMA_enviarTrama(client->fd, TIPUS_ACCEPT, FLAG_ERROR, TEXT_ITHACA_ERROR_DISPONIBILITAT);
    }

    viatge = assignacio->viatge;
    if (asprintf(&text, FORMAT_VIATGE_ACCEPTAT, viatge->id, viatge->object, viatge->destination, viatge->reward) < 0) {
        UTILS_printF(1, MSG_SENSE_MEMORIA);
        return TRAMA_enviarTrama(client->fd, TIPUS_ACCEPT, FLAG_ERROR, TEXT_ITHACA_ERROR_SERVIDOR);
    }
    assignacio->nom_odysseus = malloc(strlen(client->nom) + 1);
    if (assignacio->nom_odysseus == NULL) {
        free(text);
        UTILS_printF(1, MSG_SENSE_MEMORIA);
        return TRAMA_enviarTrama(client->fd, TIPUS_ACCEPT, FLAG_ERROR, TEXT_ITHACA_ERROR_SERVIDOR);
    }

    strcpy(assignacio->nom_odysseus, client->nom);
    assignacio->estat = VIATGE_ASSIGNAT;
    client->viatge_acceptat = 1;
    resultat = TRAMA_enviarTrama(client->fd, TIPUS_ACCEPT, FLAG_OK, text);
    free(text);

    if (asprintf(&missatge, MSG_VIATGE_ASSIGNAT, viatge->id, client->nom) >= 0) {
        UTILS_printF(1, missatge);
        free(missatge);
    } else {
        UTILS_printF(1, MSG_SENSE_MEMORIA);
    }

    return resultat;
}


int VOYAGES_acceptarClientIthaca (ServidorIthaca *servidor, int fd_server) {
    ClientIthaca *nous_clients = NULL;
    int fd_client;

    fd_client = accept(fd_server, NULL, NULL);
    if (fd_client == ERR_CODI) {
        UTILS_printF(1, MSG_ERROR_ACCEPT);
        return ERR_CODI;
    }

    if (fd_client >= FD_SETSIZE) {
        UTILS_printF(1, MSG_ERROR_DESCRIPTOR);
        close(fd_client);
        return ERR_CODI;
    }

    nous_clients = realloc(servidor->clients, sizeof(ClientIthaca) * (servidor->n_clients + 1));

    if (nous_clients == NULL) {
        UTILS_printF(1, MSG_SENSE_MEMORIA);
        close(fd_client);
        return ERR_CODI;
    }

    servidor->clients = nous_clients;
    servidor->clients[servidor->n_clients].fd = fd_client;
    servidor->clients[servidor->n_clients].nom = NULL;
    servidor->clients[servidor->n_clients].viatge_acceptat = 0;
    servidor->clients[servidor->n_clients].viatges_consultats = NULL;
    servidor->clients[servidor->n_clients].n_viatges_consultats = 0;
    servidor->n_clients++;

    return 0;
}


int VOYAGES_prepararSetIthaca (fd_set *conjunt, int fd_server, ServidorIthaca *servidor) {
    int i;
    int max_fd = fd_server;

    FD_ZERO(conjunt);
    FD_SET(fd_server, conjunt);

    for (i = 0; i < servidor->n_clients; i++) {
        FD_SET(servidor->clients[i].fd, conjunt);

        if (servidor->clients[i].fd > max_fd) {
            max_fd = servidor->clients[i].fd;
        }
    }

    return max_fd;
}


void VOYAGES_treureClientIthaca (ServidorIthaca *servidor, int index) {
    int i;

    close(servidor->clients[index].fd);
    free(servidor->clients[index].nom);
    free(servidor->clients[index].viatges_consultats);

    for (i = index; i < servidor->n_clients - 1; i++) {
        servidor->clients[i] = servidor->clients[i + 1];
    }

    servidor->n_clients--;
}


void VOYAGES_alliberarServidorIthaca (ServidorIthaca *servidor) {
    int i = 0;

    while (servidor->n_clients > 0) {
        VOYAGES_treureClientIthaca(servidor, servidor->n_clients - 1);
    }

    free(servidor->clients);
    servidor->clients = NULL;

    for (i = 0; i < servidor->n_viatges; i++) {
        free(servidor->viatges[i].nom_odysseus);
    }

    free(servidor->viatges);
    servidor->viatges = NULL;
    servidor->n_viatges = 0;

}


int VOYAGES_existeixNomIthaca (ServidorIthaca *servidor, char *nom) {
    int i;
    int existeix = 0;

    for (i = 0; i < servidor->n_clients; i++) {
        if (servidor->clients[i].nom != NULL) {
            if (strcmp(servidor->clients[i].nom, nom) == 0) {
                existeix = 1;
            }
        }
    }

    return existeix;
}


int VOYAGES_atendreConnectIthaca (ServidorIthaca *servidor, int index, char *dades) {
    ClientIthaca *client = NULL;
    char *missatge = NULL;
    int resultat;

    client = &servidor->clients[index];

    if (client->nom != NULL) {
        return TRAMA_enviarTrama(client->fd, TIPUS_CONNECT, FLAG_ERROR, TEXT_ITHACA_ERROR_ESTAT);
    }

    if (strlen(dades) == 0) {
        return TRAMA_enviarTrama(client->fd, TIPUS_CONNECT, FLAG_ERROR, TEXT_ITHACA_ERROR_DADES);
    }

    if (VOYAGES_existeixNomIthaca(servidor, dades) == 1) {
        return TRAMA_enviarTrama(client->fd, TIPUS_CONNECT, FLAG_ERROR, TEXT_ITHACA_ERROR_NOM);
    }

    client->nom = malloc(strlen(dades) + 1);
    if (client->nom == NULL) {
        UTILS_printF(1, MSG_SENSE_MEMORIA);
        return TRAMA_enviarTrama(client->fd, TIPUS_CONNECT, FLAG_ERROR, TEXT_ITHACA_ERROR_SERVIDOR);
    }

    strcpy(client->nom, dades);

    resultat = TRAMA_enviarTrama(client->fd, TIPUS_CONNECT, FLAG_OK, TEXT_ITHACA_OK);

    if (resultat == TRAMA_OK) {
        if (asprintf(&missatge, MSG_CLIENT_CONNECTAT, client->nom) >= 0) {
            UTILS_printF(1, missatge);
            free(missatge);
        } else {
            UTILS_printF(1, MSG_SENSE_MEMORIA);
        }
    }

    return resultat;
}


void VOYAGES_atendreTramaIthaca (ServidorIthaca *servidor, int index) {
    char *dades = NULL;
    char *missatge = NULL;
    int tipus;
    int flags;
    int fd;
    int resultat;
    int desconnectar = 0;

    fd = servidor->clients[index].fd;

    if (TRAMA_rebreTrama(fd, &tipus, &flags, &dades) == TRAMA_ERROR) {
        free(dades);
        VOYAGES_treureClientIthaca(servidor, index);
        return;
    }

    if (flags != FLAG_PETICIO) {
        resultat = TRAMA_enviarTrama(fd, TIPUS_NACK, FLAG_ERROR, TEXT_ITHACA_ERROR_FLAGS);
    } else if (tipus == TIPUS_CONNECT) {
        resultat = VOYAGES_atendreConnectIthaca(servidor, index, dades);
    } else if (tipus == TIPUS_LIST_VOYAGES) {
        resultat = VOYAGES_atendreListVoyages(servidor, index, dades);
    } else if (tipus == TIPUS_ACCEPT) {
        resultat = VOYAGES_atendreAcceptIthaca(servidor, index, dades);
    } else if (tipus == TIPUS_SORTIR_ITHACA) {
        if (servidor->clients[index].nom == NULL) {
            resultat = TRAMA_enviarTrama(fd, tipus, FLAG_ERROR, TEXT_ITHACA_ERROR_ESTAT);
        } else if (strlen(dades) != 0) {
            resultat = TRAMA_enviarTrama(fd, tipus, FLAG_ERROR, TEXT_ITHACA_ERROR_DADES);
        } else {
            resultat = TRAMA_enviarTrama(fd, tipus, FLAG_OK, TEXT_ITHACA_OK);
            asprintf(&missatge, MSG_CLIENT_MARXA, servidor->clients[index].nom);
            UTILS_printF(1, missatge);
            free(missatge);
            
            desconnectar = 1;
        }
    } else {
        resultat = TRAMA_enviarTrama(fd, TIPUS_NACK, FLAG_ERROR, TEXT_ITHACA_ERROR_TIPUS);
    }

    free(dades);

    if (resultat == TRAMA_ERROR || desconnectar == 1) {
        VOYAGES_treureClientIthaca(servidor, index);
    }
}

int VOYAGES_executarServidorIthaca (int fd_server, ServidorIthaca *servidor, int *sortir) {
    fd_set conjunt;
    int max_fd;
    int resultat;
    int i;

    if (fd_server < 0 || fd_server >= FD_SETSIZE) {
        UTILS_printF(1, MSG_ERROR_DESCRIPTOR);
        return ERR_CODI;
    }

    while (*sortir == 0) {
        max_fd = VOYAGES_prepararSetIthaca(&conjunt, fd_server, servidor);

        resultat = select(max_fd + 1, &conjunt, NULL, NULL, NULL);

        if (resultat < 0) {
            if (*sortir == 0) {
                UTILS_printF(1, MSG_ERROR_SELECT);
                return ERR_CODI;
            }
        } else if (*sortir == 0) {
            for (i = servidor->n_clients - 1; i >= 0; i--) {
                if (FD_ISSET(servidor->clients[i].fd, &conjunt)) {
                    VOYAGES_atendreTramaIthaca(servidor, i);
                }
            }

            if (FD_ISSET(fd_server, &conjunt)) {
                VOYAGES_acceptarClientIthaca(servidor, fd_server);
            }
        }
    }

    return 0;
}

