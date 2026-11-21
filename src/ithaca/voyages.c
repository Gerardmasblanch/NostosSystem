/***********************************************
*
* @Proposit: Implementacio de la gestio dels clients i dels viatges del servidor Ithaca.
* @Autor/s: Arnau Ricart i Gerard Mas
* @Data creacio: 29/10/2026
* @Data ultima modificacio: 20/11/2026
*
************************************************/

#include "voyages.h"


void VOYAGES_inicialitzarServidorIthaca (ServidorIthaca *servidor, char *carpeta, int fd_avis) {
    servidor->viatges = NULL;
    servidor->n_viatges = 0;
    servidor->clients = NULL;
    servidor->n_clients = 0;
    servidor->fd_avis = fd_avis;
    servidor->carpeta = carpeta;

    pthread_mutex_init(&servidor->mutex_viatges, NULL);
    pthread_mutex_init(&servidor->mutex_clients, NULL);
}


void VOYAGES_alliberarClient (ClientIthaca *client) {
    free(client->nom);
    free(client->viatges_consultats);
    free(client);
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


int VOYAGES_guardarViatgesConsultats (ServidorIthaca *servidor, ClientIthaca *client) {
    int total;
    int i;

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


int VOYAGES_copiarViatgesDisponibles (ServidorIthaca *servidor, char ***textos) {
    Voyage *viatge = NULL;
    int total;
    int numero = 0;
    int i;

    *textos = NULL;
    total = VOYAGES_comptarViatgesDisponibles(servidor);

    if (total == 0) {
        return 0;
    }

    *textos = malloc(sizeof(char *) * total);
    if (*textos == NULL) {
        UTILS_printF(1, MSG_SENSE_MEMORIA);
        return ERR_CODI;
    }

    for (i = 0; i < servidor->n_viatges; i++) {
        if (servidor->viatges[i].estat == VIATGE_DISPONIBLE) {
            viatge = servidor->viatges[i].viatge;

            if (asprintf(&(*textos)[numero], FORMAT_VIATGE_DISPONIBLE, numero + 1, total, viatge->id, viatge->object, viatge->destination, viatge->reward) < 0) {
                UTILS_printF(1, MSG_SENSE_MEMORIA);
                VOYAGES_alliberarTextos(*textos, numero);
                *textos = NULL;
                return ERR_CODI;
            }
            numero++;
        }
    }

    return total;
}


int VOYAGES_enviarViatges (int fd, char **textos, int n_textos) {
    int i;
    int resultat = TRAMA_OK;

    if (n_textos == 0) {
        return TRAMA_enviarTrama(fd, TIPUS_LIST_VOYAGES, FLAG_OK, TEXT_ITHACA_SENSE_VIATGES);
    }

    for (i = 0; i < n_textos && resultat == TRAMA_OK; i++) {
        resultat = TRAMA_enviarTrama(fd, TIPUS_LIST_VOYAGES, FLAG_OK, textos[i]);
    }

    return resultat;
}


void VOYAGES_alliberarTextos (char **textos, int n_textos) {
    int i;

    for (i = 0; i < n_textos; i++) {
        free(textos[i]);
    }
    free(textos);
}


int VOYAGES_atendreListVoyages (ClientIthaca *client, char *dades) {
    ServidorIthaca *servidor = client->servidor;
    char **textos = NULL;
    char *missatge = NULL;
    int n_textos = 0;
    int index_viatge;
    int id_fracassat = ERR_CODI;
    int error = 0;
    int resultat;

    if (client->nom == NULL || client->viatge_acceptat == 1) {
        return TRAMA_enviarTrama(client->fd, TIPUS_LIST_VOYAGES, FLAG_ERROR, TEXT_ITHACA_ERROR_ESTAT);
    }

    if (strlen(dades) != 0) {
        return TRAMA_enviarTrama(client->fd, TIPUS_LIST_VOYAGES, FLAG_ERROR, TEXT_ITHACA_ERROR_DADES);
    }

    pthread_mutex_lock(&servidor->mutex_viatges);

    index_viatge = VOYAGES_buscarViatgeAssignat(servidor, client->nom);
    if (index_viatge != ERR_CODI) {
        servidor->viatges[index_viatge].estat = VIATGE_FRACASSAT;
        id_fracassat = servidor->viatges[index_viatge].viatge->id;
    }

    if (VOYAGES_guardarViatgesConsultats(servidor, client) == ERR_CODI) {
        error = 1;
    } else {
        n_textos = VOYAGES_copiarViatgesDisponibles(servidor, &textos);
        if (n_textos == ERR_CODI) {
            error = 1;
        }
    }

    pthread_mutex_unlock(&servidor->mutex_viatges);

    if (id_fracassat != ERR_CODI) {
        if (asprintf(&missatge, MSG_VIATGE_FRACASSAT, id_fracassat, client->nom) >= 0) {
            UTILS_printF(1, missatge);
            free(missatge);
            missatge = NULL;
        } else {
            UTILS_printF(1, MSG_SENSE_MEMORIA);
        }
    }

    if (error == 1) {
        return TRAMA_enviarTrama(client->fd, TIPUS_LIST_VOYAGES, FLAG_ERROR, TEXT_ITHACA_ERROR_SERVIDOR);
    }

    if (asprintf(&missatge, MSG_CONSULTA_VIATGES, client->nom) >= 0) {
        UTILS_printF(1, missatge);
        free(missatge);
    } else {
        UTILS_printF(1, MSG_SENSE_MEMORIA);
    }

    resultat = VOYAGES_enviarViatges(client->fd, textos, n_textos);
    VOYAGES_alliberarTextos(textos, n_textos);

    return resultat;
}


int VOYAGES_atendreAcceptIthaca (ClientIthaca *client, char *dades) {
    ServidorIthaca *servidor = client->servidor;
    ViatgeIthaca *assignacio = NULL;
    Voyage *viatge = NULL;
    char *text = NULL;
    char *missatge = NULL;
    char *error = NULL;
    int index_viatge;
    int id_viatge;
    int resultat;

    if (client->nom == NULL || client->viatge_acceptat == 1) {
        return TRAMA_enviarTrama(client->fd, TIPUS_ACCEPT, FLAG_ERROR, TEXT_ITHACA_ERROR_ESTAT);
    }
    if (UTILS_esNumeric(dades) == TEXT_NO_NUMERIC) {
        return TRAMA_enviarTrama(client->fd, TIPUS_ACCEPT, FLAG_ERROR, TEXT_ITHACA_ERROR_DADES);
    }

    id_viatge = atoi(dades);
    if (VOYAGES_haConsultatViatge(client, id_viatge) == 0) {
        return TRAMA_enviarTrama(client->fd, TIPUS_ACCEPT, FLAG_ERROR, TEXT_ITHACA_ERROR_DADES);
    }

    pthread_mutex_lock(&servidor->mutex_viatges);

    index_viatge = VOYAGES_buscarViatgePerId(servidor, id_viatge);

    if (VOYAGES_buscarViatgeAssignat(servidor, client->nom) != ERR_CODI) {
        error = TEXT_ITHACA_ERROR_ESTAT;
    } else if (index_viatge == ERR_CODI) {
        error = TEXT_ITHACA_ERROR_DADES;
    } else if (servidor->viatges[index_viatge].estat != VIATGE_DISPONIBLE) {
        error = TEXT_ITHACA_ERROR_DISPONIBILITAT;
    } else {
        assignacio = &servidor->viatges[index_viatge];
        viatge = assignacio->viatge;
        assignacio->nom_odysseus = malloc(strlen(client->nom) + 1);

        if (assignacio->nom_odysseus == NULL) {
            error = TEXT_ITHACA_ERROR_SERVIDOR;
        } else if (asprintf(&text, FORMAT_VIATGE_ACCEPTAT, viatge->id, viatge->object, viatge->destination, viatge->reward) < 0) {
            free(assignacio->nom_odysseus);
            assignacio->nom_odysseus = NULL;
            error = TEXT_ITHACA_ERROR_SERVIDOR;
        } else {
            strcpy(assignacio->nom_odysseus, client->nom);
            assignacio->estat = VIATGE_ASSIGNAT;
        }
    }

    pthread_mutex_unlock(&servidor->mutex_viatges);

    if (error != NULL) {
        return TRAMA_enviarTrama(client->fd, TIPUS_ACCEPT, FLAG_ERROR, error);
    }

    client->viatge_acceptat = 1;
    resultat = TRAMA_enviarTrama(client->fd, TIPUS_ACCEPT, FLAG_OK, text);
    free(text);

    if (resultat == TRAMA_ERROR) {
        return TRAMA_ERROR;
    }

    if (asprintf(&missatge, MSG_VIATGE_ASSIGNAT, viatge->id, client->nom) >= 0) {
        UTILS_printF(1, missatge);
        free(missatge);
        missatge = NULL;
    } else {
        UTILS_printF(1, MSG_SENSE_MEMORIA);
    }

    return VOYAGES_enviarObjecte(client, index_viatge);
}


int VOYAGES_enviarObjecte (ClientIthaca *client, int index_viatge) {
    ServidorIthaca *servidor = client->servidor;
    Voyage *viatge = servidor->viatges[index_viatge].viatge;
    char *missatge = NULL;
    int resultat;

    asprintf(&missatge, MSG_ENVIANT_OBJECTE, viatge->object, client->nom);
    UTILS_printF(1, missatge);
    free(missatge);
    missatge = NULL;

    resultat = FITXER_enviar(client->fd, servidor->fd_avis, viatge->file, UTILS_nomFitxer(viatge->file), TEXT_FITXER_OBJECT, 1);

    if (resultat == FITXER_OK) {
        UTILS_printF(1, MSG_OBJECTE_ENVIAT);
        return TRAMA_OK;
    }

    pthread_mutex_lock(&servidor->mutex_viatges);
    servidor->viatges[index_viatge].estat = VIATGE_FRACASSAT;
    pthread_mutex_unlock(&servidor->mutex_viatges);

    client->viatge_acceptat = 0;

    asprintf(&missatge, MSG_OBJECTE_FALLAT, client->nom, viatge->id);
    UTILS_printF(1, missatge);
    free(missatge);

    if (resultat == FITXER_ERROR) {
        return TRAMA_ERROR;
    }

    return TRAMA_OK;
}


int VOYAGES_rebutjarClaim (ClientIthaca *client, char *id_text, char *motiu) {
    char *missatge = NULL;

    asprintf(&missatge, MSG_CLAIM_REBUTJAT, id_text, client->nom);
    UTILS_printF(1, missatge);
    free(missatge);

    return TRAMA_enviarTrama(client->fd, TIPUS_CLAIM, FLAG_ERROR, motiu);
}


int VOYAGES_esObjecteDelViatge (Voyage *viatge, char *path_rebut) {
    char md5_rebut[MIDA_MD5 + 1];
    char md5_esperat[MIDA_MD5 + 1];
    int resultat = 0;

    if (FITXER_calcularMD5(path_rebut, md5_rebut) == FITXER_OK &&
        SPHRAGIS_sign_with_glyph(viatge->destination, viatge->file, path_rebut) == SPHRAGIS_OK &&
        FITXER_calcularMD5(path_rebut, md5_esperat) == FITXER_OK &&
        strcmp(md5_rebut, md5_esperat) == 0) {
        resultat = 1;
    }

    return resultat;
}

int VOYAGES_atendreClaim (ClientIthaca *client, char *dades) {
    ServidorIthaca *servidor = client->servidor;
    Voyage *viatge = NULL;
    char *path_rebut = NULL;
    char *missatge = NULL;
    char *text = NULL;
    char *error = NULL;
    int index_viatge;
    int resultat;

    if (client->nom == NULL) {
        TRAMA_enviarTrama(client->fd, TIPUS_CLAIM, FLAG_ERROR, TEXT_ITHACA_ERROR_ESTAT);
        return TRAMA_ERROR;
    }
    if (UTILS_esNumeric(dades) == TEXT_NO_NUMERIC) {
        TRAMA_enviarTrama(client->fd, TIPUS_CLAIM, FLAG_ERROR, TEXT_ITHACA_ERROR_DADES);
        return TRAMA_ERROR;
    }

    asprintf(&missatge, MSG_CLAIM_PETICIO, client->nom, dades);
    UTILS_printF(1, missatge);
    free(missatge);
    missatge = NULL;

    pthread_mutex_lock(&servidor->mutex_viatges);
    index_viatge = VOYAGES_buscarViatgePerId(servidor, atoi(dades));
    pthread_mutex_unlock(&servidor->mutex_viatges);

    if (index_viatge != ERR_CODI) {
        viatge = servidor->viatges[index_viatge].viatge;
        asprintf(&missatge, MSG_CLAIM_REBENT, viatge->object);
        UTILS_printF(1, missatge);
        free(missatge);
        missatge = NULL;
    }

    resultat = FITXER_rebre(client->fd, servidor->fd_avis, servidor->carpeta, TEXT_FITXER_CLAIM, PREFIX_CLAIM, &path_rebut);
    if (resultat == FITXER_ERROR) {
        return TRAMA_ERROR;
    }
    if (resultat != FITXER_OK) {
        return VOYAGES_rebutjarClaim(client, dades, TEXT_ITHACA_ERROR_TRANSFERENCIA);
    }
    UTILS_printF(1, MSG_CLAIM_REBUT);

    if (viatge == NULL) {
        error = TEXT_ITHACA_ERROR_DADES;
    } else if (strcmp(UTILS_nomFitxer(path_rebut) + strlen(PREFIX_CLAIM), UTILS_nomFitxer(viatge->file)) != 0) {
        error = TEXT_ITHACA_ERROR_DADES;
    } else if (SPHRAGIS_has_glyph(viatge->destination, path_rebut) != 1) {
        error = TEXT_ITHACA_ERROR_GLIF;
    } else if (VOYAGES_esObjecteDelViatge(viatge, path_rebut) == 0) {
        error = TEXT_ITHACA_ERROR_OBJECTE;
    } else {
        pthread_mutex_lock(&servidor->mutex_viatges);
        if (servidor->viatges[index_viatge].estat == VIATGE_ASSIGNAT &&
            strcmp(servidor->viatges[index_viatge].nom_odysseus, client->nom) == 0) {
            servidor->viatges[index_viatge].estat = VIATGE_COMPLETAT;
        } else {
            error = TEXT_ITHACA_ERROR_ESTAT;
        }
        pthread_mutex_unlock(&servidor->mutex_viatges);
    }

    if (error != NULL) {
        FITXER_esborrar(path_rebut);
    }
    free(path_rebut);

    if (error != NULL) {
        return VOYAGES_rebutjarClaim(client, dades, error);
    }

    asprintf(&missatge, MSG_CLAIM_GLIF, viatge->destination);
    UTILS_printF(1, missatge);
    free(missatge);
    missatge = NULL;

    asprintf(&text, FORMAT_CLAIM_OK, viatge->reward);
    resultat = TRAMA_enviarTrama(client->fd, TIPUS_CLAIM, FLAG_OK, text);
    free(text);

    asprintf(&missatge, MSG_CLAIM_COMPLETAT, viatge->id, client->nom);
    UTILS_printF(1, missatge);
    free(missatge);
    missatge = NULL;

    asprintf(&missatge, MSG_CLAIM_RECOMPENSA, viatge->reward);
    UTILS_printF(1, missatge);
    free(missatge);

    client->viatge_acceptat = 0;

    return resultat;
}


int VOYAGES_acceptarClientIthaca (ServidorIthaca *servidor, int fd_server) {
    ClientIthaca *client = NULL;
    ClientIthaca **nous_clients = NULL;
    int fd_client;

    fd_client = accept(fd_server, NULL, NULL);
    if (fd_client < 0) {
        UTILS_printF(1, MSG_ERROR_ACCEPT);
        return ERR_CODI;
    }

    if (fd_client >= FD_SETSIZE) {
        UTILS_printF(1, MSG_ERROR_DESCRIPTOR);
        close(fd_client);
        return ERR_CODI;
    }

    client = malloc(sizeof(ClientIthaca));
    if (client == NULL) {
        UTILS_printF(1, MSG_SENSE_MEMORIA);
        close(fd_client);
        return ERR_CODI;
    }

    client->servidor = servidor;
    client->fd = fd_client;
    client->nom = NULL;
    client->viatge_acceptat = 0;
    client->viatges_consultats = NULL;
    client->n_viatges_consultats = 0;
    client->acabat = 0;

    pthread_mutex_lock(&servidor->mutex_clients);

    nous_clients = realloc(servidor->clients, sizeof(ClientIthaca *) * (servidor->n_clients + 1));
    if (nous_clients == NULL) {
        pthread_mutex_unlock(&servidor->mutex_clients);
        UTILS_printF(1, MSG_SENSE_MEMORIA);
        close(fd_client);
        free(client);
        return ERR_CODI;
    }

    servidor->clients = nous_clients;
    servidor->clients[servidor->n_clients] = client;
    servidor->n_clients++;

    if (pthread_create(&client->fil, NULL, VOYAGES_atendreClient, client) != 0) {
        servidor->n_clients--;
        pthread_mutex_unlock(&servidor->mutex_clients);
        UTILS_printF(1, MSG_ERROR_THREAD);
        close(fd_client);
        free(client);
        return ERR_CODI;
    }

    pthread_mutex_unlock(&servidor->mutex_clients);

    return 0;
}


void VOYAGES_netejarClientsAcabats (ServidorIthaca *servidor) {
    ClientIthaca *client = NULL;
    int i = 0;
    int j;

    pthread_mutex_lock(&servidor->mutex_clients);

    while (i < servidor->n_clients) {
        if (servidor->clients[i]->acabat == 1) {
            client = servidor->clients[i];

            for (j = i; j < servidor->n_clients - 1; j++) {
                servidor->clients[j] = servidor->clients[j + 1];
            }
            servidor->n_clients--;

            pthread_join(client->fil, NULL);
            VOYAGES_alliberarClient(client);
        } else {
            i++;
        }
    }

    pthread_mutex_unlock(&servidor->mutex_clients);
}


void VOYAGES_esperarClients (ServidorIthaca *servidor) {
    int i;

    for (i = 0; i < servidor->n_clients; i++) {
        pthread_join(servidor->clients[i]->fil, NULL);
    }
}


void VOYAGES_alliberarServidorIthaca (ServidorIthaca *servidor) {
    int i;

    for (i = 0; i < servidor->n_clients; i++) {
        VOYAGES_alliberarClient(servidor->clients[i]);
    }
    free(servidor->clients);
    servidor->clients = NULL;
    servidor->n_clients = 0;

    for (i = 0; i < servidor->n_viatges; i++) {
        free(servidor->viatges[i].nom_odysseus);
    }
    free(servidor->viatges);
    servidor->viatges = NULL;
    servidor->n_viatges = 0;

    pthread_mutex_destroy(&servidor->mutex_viatges);
    pthread_mutex_destroy(&servidor->mutex_clients);
}


int VOYAGES_existeixNomIthaca (ServidorIthaca *servidor, char *nom) {
    int i;
    int existeix = 0;

    for (i = 0; i < servidor->n_clients; i++) {
        if (servidor->clients[i]->acabat == 0 && servidor->clients[i]->nom != NULL) {
            if (strcmp(servidor->clients[i]->nom, nom) == 0) {
                existeix = 1;
            }
        }
    }

    return existeix;
}


int VOYAGES_atendreConnectIthaca (ClientIthaca *client, char *dades) {
    ServidorIthaca *servidor = client->servidor;
    char *nom = NULL;
    char *missatge = NULL;
    char *format = NULL;
    int existeix;
    int index_viatge;
    int resultat;

    if (client->nom != NULL) {
        return TRAMA_enviarTrama(client->fd, TIPUS_CONNECT, FLAG_ERROR, TEXT_ITHACA_ERROR_ESTAT);
    }

    if (strlen(dades) == 0) {
        return TRAMA_enviarTrama(client->fd, TIPUS_CONNECT, FLAG_ERROR, TEXT_ITHACA_ERROR_DADES);
    }

    nom = malloc(strlen(dades) + 1);
    if (nom == NULL) {
        UTILS_printF(1, MSG_SENSE_MEMORIA);
        return TRAMA_enviarTrama(client->fd, TIPUS_CONNECT, FLAG_ERROR, TEXT_ITHACA_ERROR_SERVIDOR);
    }
    strcpy(nom, dades);

    pthread_mutex_lock(&servidor->mutex_clients);
    existeix = VOYAGES_existeixNomIthaca(servidor, nom);
    if (existeix == 0) {
        client->nom = nom;
    }
    pthread_mutex_unlock(&servidor->mutex_clients);

    if (existeix == 1) {
        free(nom);
        return TRAMA_enviarTrama(client->fd, TIPUS_CONNECT, FLAG_ERROR, TEXT_ITHACA_ERROR_NOM);
    }

    resultat = TRAMA_enviarTrama(client->fd, TIPUS_CONNECT, FLAG_OK, TEXT_ITHACA_OK);

    if (resultat == TRAMA_OK) {
        pthread_mutex_lock(&servidor->mutex_viatges);
        index_viatge = VOYAGES_buscarViatgeAssignat(servidor, client->nom);
        pthread_mutex_unlock(&servidor->mutex_viatges);

        format = MSG_CLIENT_CONNECTAT;
        if (index_viatge != ERR_CODI) {
            format = MSG_CLIENT_TORNAT;
        }

        if (asprintf(&missatge, format, client->nom) >= 0) {
            UTILS_printF(1, missatge);
            free(missatge);
        } else {
            UTILS_printF(1, MSG_SENSE_MEMORIA);
        }
    }

    return resultat;
}


int VOYAGES_atendreTramaIthaca (ClientIthaca *client) {
    char *dades = NULL;
    char *missatge = NULL;
    int tipus;
    int flags;
    int resultat;
    int continuar = CLIENT_CONTINUA;

    if (TRAMA_rebreTrama(client->fd, client->servidor->fd_avis, &tipus, &flags, &dades) == TRAMA_ERROR) {
        free(dades);
        return CLIENT_ACABA;
    }

    if (flags != FLAG_PETICIO) {
        resultat = TRAMA_enviarTrama(client->fd, TIPUS_NACK, FLAG_ERROR, TEXT_ITHACA_ERROR_FLAGS);
    } else if (tipus == TIPUS_CONNECT) {
        resultat = VOYAGES_atendreConnectIthaca(client, dades);
    } else if (tipus == TIPUS_LIST_VOYAGES) {
        resultat = VOYAGES_atendreListVoyages(client, dades);
    } else if (tipus == TIPUS_ACCEPT) {
        resultat = VOYAGES_atendreAcceptIthaca(client, dades);
    } else if (tipus == TIPUS_CLAIM) {
        resultat = VOYAGES_atendreClaim(client, dades);
    } else if (tipus == TIPUS_SORTIR_ITHACA) {
        if (client->nom == NULL) {
            resultat = TRAMA_enviarTrama(client->fd, tipus, FLAG_ERROR, TEXT_ITHACA_ERROR_ESTAT);
        } else if (strlen(dades) != 0) {
            resultat = TRAMA_enviarTrama(client->fd, tipus, FLAG_ERROR, TEXT_ITHACA_ERROR_DADES);
        } else {
            resultat = TRAMA_enviarTrama(client->fd, tipus, FLAG_OK, TEXT_ITHACA_OK);
            asprintf(&missatge, MSG_CLIENT_MARXA, client->nom);
            UTILS_printF(1, missatge);
            free(missatge);
            continuar = CLIENT_ACABA;
        }
    } else {
        resultat = TRAMA_enviarTrama(client->fd, TIPUS_NACK, FLAG_ERROR, TEXT_ITHACA_ERROR_TIPUS);
    }

    free(dades);

    if (resultat == TRAMA_ERROR) {
        continuar = CLIENT_ACABA;
    }

    return continuar;
}


void *VOYAGES_atendreClient (void *arg) {
    ClientIthaca *client = (ClientIthaca *) arg;
    ServidorIthaca *servidor = client->servidor;
    fd_set conjunt;
    int max_fd;
    int continuar = CLIENT_CONTINUA;

    max_fd = client->fd;
    if (servidor->fd_avis > max_fd) {
        max_fd = servidor->fd_avis;
    }

    while (continuar == CLIENT_CONTINUA) {
        FD_ZERO(&conjunt);
        FD_SET(client->fd, &conjunt);
        FD_SET(servidor->fd_avis, &conjunt);

        if (select(max_fd + 1, &conjunt, NULL, NULL, NULL) > 0) {
            if (FD_ISSET(servidor->fd_avis, &conjunt)) {
                continuar = CLIENT_ACABA;
            } else if (FD_ISSET(client->fd, &conjunt)) {
                continuar = VOYAGES_atendreTramaIthaca(client);
            }
        }
    }

    close(client->fd);

    pthread_mutex_lock(&servidor->mutex_clients);
    client->acabat = 1;
    pthread_mutex_unlock(&servidor->mutex_clients);

    return NULL;
}


int VOYAGES_executarServidorIthaca (int fd_server, ServidorIthaca *servidor) {
    fd_set conjunt;
    int max_fd;
    int sortir = 0;

    if (fd_server >= FD_SETSIZE || servidor->fd_avis >= FD_SETSIZE) {
        UTILS_printF(1, MSG_ERROR_DESCRIPTOR);
        return ERR_CODI;
    }

    max_fd = fd_server;
    if (servidor->fd_avis > max_fd) {
        max_fd = servidor->fd_avis;
    }

    while (sortir == 0) {
        FD_ZERO(&conjunt);
        FD_SET(fd_server, &conjunt);
        FD_SET(servidor->fd_avis, &conjunt);

        if (select(max_fd + 1, &conjunt, NULL, NULL, NULL) > 0) {
            if (FD_ISSET(servidor->fd_avis, &conjunt)) {
                sortir = 1;
            } else if (FD_ISSET(fd_server, &conjunt)) {
                VOYAGES_acceptarClientIthaca(servidor, fd_server);
            }
        }

        VOYAGES_netejarClientsAcabats(servidor);
    }

    VOYAGES_esperarClients(servidor);

    return 0;
}
