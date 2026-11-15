/***********************************************
*
* @Proposit: Implementacio del servidor de l'illa, connexions, port i cua
*            d'espera. Cada vaixell s'atén en un thread propi.
* @Autor/s: Arnau Ricart i Gerard Mas
* @Data creacio: 29/10/2026
* @Data ultima modificacio: 15/11/2026
*
************************************************/

//include del fitxer .h del mateix mòdul
#include "harbor.h"


//funcions i procediments

void HARBOR_inicialitzarHarbor (Harbor *harbor, IslandConfig *config, Mercat *mercat, int fd_avis) {
    harbor->config = config;
    harbor->mercat = mercat;
    harbor->vaixells = NULL;
    harbor->n_vaixells = 0;
    harbor->cua = NULL;
    harbor->n_cua = 0;
    harbor->n_atracats = 0;
    harbor->fd_avis = fd_avis;

    pthread_mutex_init(&harbor->mutex_vaixells, NULL);
    pthread_mutex_init(&harbor->mutex_port, NULL);
}

void HARBOR_alliberarVaixell (Vaixell *vaixell) {
    free(vaixell->nom);
    free(vaixell);
}

int HARBOR_acceptarVaixell (Harbor *harbor, int fd_servidor) {
    Vaixell *vaixell = NULL;
    Vaixell **nous_vaixells = NULL;
    int fd_vaixell;

    fd_vaixell = accept(fd_servidor, NULL, NULL);
    if (fd_vaixell < 0) {
        UTILS_printF(1, MSG_ERROR_ACCEPT);
        return ERR_CODI;
    }

    if (fd_vaixell >= FD_SETSIZE) {
        UTILS_printF(1, MSG_ERROR_DESCRIPTOR);
        close(fd_vaixell);
        return ERR_CODI;
    }

    vaixell = malloc(sizeof(Vaixell));
    if (vaixell == NULL) {
        UTILS_printF(1, MSG_SENSE_MEMORIA);
        close(fd_vaixell);
        return ERR_CODI;
    }

    vaixell->harbor = harbor;
    vaixell->fd = fd_vaixell;
    vaixell->nom = NULL;
    vaixell->estat = VAIXELL_CONNECTAT;
    vaixell->acabat = 0;

    pthread_mutex_lock(&harbor->mutex_vaixells);

    nous_vaixells = realloc(harbor->vaixells, sizeof(Vaixell *) * (harbor->n_vaixells + 1));
    if (nous_vaixells == NULL) {
        pthread_mutex_unlock(&harbor->mutex_vaixells);
        UTILS_printF(1, MSG_SENSE_MEMORIA);
        close(fd_vaixell);
        free(vaixell);
        return ERR_CODI;
    }

    harbor->vaixells = nous_vaixells;
    harbor->vaixells[harbor->n_vaixells] = vaixell;
    harbor->n_vaixells++;

    if (pthread_create(&vaixell->fil, NULL, HARBOR_atendreVaixell, vaixell) != 0) {
        harbor->n_vaixells--;
        pthread_mutex_unlock(&harbor->mutex_vaixells);
        UTILS_printF(1, MSG_ERROR_THREAD);
        close(fd_vaixell);
        free(vaixell);
        return ERR_CODI;
    }

    pthread_mutex_unlock(&harbor->mutex_vaixells);

    return 0;
}

void HARBOR_netejarVaixellsAcabats (Harbor *harbor) {
    Vaixell *vaixell = NULL;
    int i = 0;
    int j;

    pthread_mutex_lock(&harbor->mutex_vaixells);

    while (i < harbor->n_vaixells) {
        if (harbor->vaixells[i]->acabat == 1) {
            vaixell = harbor->vaixells[i];

            for (j = i; j < harbor->n_vaixells - 1; j++) {
                harbor->vaixells[j] = harbor->vaixells[j + 1];
            }
            harbor->n_vaixells--;

            pthread_join(vaixell->fil, NULL);
            HARBOR_alliberarVaixell(vaixell);
        } else {
            i++;
        }
    }

    pthread_mutex_unlock(&harbor->mutex_vaixells);
}

void HARBOR_esperarVaixells (Harbor *harbor) {
    int i;

    for (i = 0; i < harbor->n_vaixells; i++) {
        pthread_join(harbor->vaixells[i]->fil, NULL);
    }
}

void HARBOR_treureDeLaCua (Harbor *harbor, int posicio) {
    int i;

    for (i = posicio; i < harbor->n_cua - 1; i++) {
        harbor->cua[i] = harbor->cua[i + 1];
    }
    harbor->n_cua--;
}

int HARBOR_afegirALaCua (Harbor *harbor, Vaixell *vaixell) {
    Vaixell **nova_cua = NULL;

    nova_cua = realloc(harbor->cua, sizeof(Vaixell *) * (harbor->n_cua + 1));
    if (nova_cua == NULL) {
        UTILS_printF(1, MSG_SENSE_MEMORIA);
        return ERR_CODI;
    }

    harbor->cua = nova_cua;
    harbor->cua[harbor->n_cua] = vaixell;
    harbor->n_cua++;

    return 0;
}

void HARBOR_atracarSeguent (Harbor *harbor) {
    Vaixell *seguent = NULL;
    char *missatge = NULL;

    while (harbor->n_atracats < harbor->config->capacity && harbor->n_cua > 0) {
        seguent = harbor->cua[0];
        HARBOR_treureDeLaCua(harbor, 0);

        seguent->estat = VAIXELL_ATRACAT;
        harbor->n_atracats++;
        TRAMA_enviarTrama(seguent->fd, TIPUS_ARRIBADA, FLAG_OK, TEXT_DOCKED);

        asprintf(&missatge, MSG_DOCKED_ENVIAT, seguent->nom);
        UTILS_printF(1, missatge);
        free(missatge);
    }
}

void HARBOR_atendreArribada (Vaixell *vaixell, char *dades) {
    Harbor *harbor = vaixell->harbor;
    char *missatge = NULL;

    if (vaixell->nom != NULL) {
        TRAMA_enviarTrama(vaixell->fd, TIPUS_ARRIBADA, FLAG_ERROR, TEXT_ERROR_ESTAT);
        return;
    }

    if (strlen(dades) == 0) {
        TRAMA_enviarTrama(vaixell->fd, TIPUS_ARRIBADA, FLAG_ERROR, TEXT_ERROR_DADES);
        return;
    }

    vaixell->nom = malloc(sizeof(char) * (strlen(dades) + 1));
    if (vaixell->nom == NULL) {
        UTILS_printF(1, MSG_SENSE_MEMORIA);
        TRAMA_enviarTrama(vaixell->fd, TIPUS_ARRIBADA, FLAG_ERROR, TEXT_ERROR_SERVIDOR);
        return;
    }
    strcpy(vaixell->nom, dades);

    asprintf(&missatge, MSG_APROXIMANT, vaixell->nom, harbor->config->name);
    UTILS_printF(1, missatge);
    free(missatge);
    missatge = NULL;

    pthread_mutex_lock(&harbor->mutex_port);

    if (harbor->n_atracats < harbor->config->capacity) {
        vaixell->estat = VAIXELL_ATRACAT;
        harbor->n_atracats++;
        TRAMA_enviarTrama(vaixell->fd, TIPUS_ARRIBADA, FLAG_OK, TEXT_DOCKED);
        asprintf(&missatge, MSG_ATRACAT, vaixell->nom);
    } else if (HARBOR_afegirALaCua(harbor, vaixell) == 0) {
        vaixell->estat = VAIXELL_ESPERANT;
        TRAMA_enviarTrama(vaixell->fd, TIPUS_ARRIBADA, FLAG_OK, TEXT_WAIT);
        asprintf(&missatge, MSG_PORT_PLE, vaixell->nom);
    } else {
        TRAMA_enviarTrama(vaixell->fd, TIPUS_ARRIBADA, FLAG_ERROR, TEXT_ERROR_SERVIDOR);
    }

    pthread_mutex_unlock(&harbor->mutex_port);

    if (missatge != NULL) {
        UTILS_printF(1, missatge);
        free(missatge);
    }
}

int HARBOR_atendreSortida (Vaixell *vaixell) {
    if (vaixell->nom == NULL) {
        TRAMA_enviarTrama(vaixell->fd, TIPUS_SORTIR_ILLA, FLAG_ERROR, TEXT_ERROR_ESTAT);
        return VAIXELL_CONTINUA;
    }

    TRAMA_enviarTrama(vaixell->fd, TIPUS_SORTIR_ILLA, FLAG_OK, TEXT_OK);
    return VAIXELL_ACABA;
}

void HARBOR_ferSortir (Vaixell *vaixell, int tancant_illa) {
    Harbor *harbor = vaixell->harbor;
    char *missatge = NULL;
    int i = 0;
    int trobat = 0;

    pthread_mutex_lock(&harbor->mutex_port);

    if (vaixell->estat == VAIXELL_ATRACAT) {
        harbor->n_atracats--;
    } else if (vaixell->estat == VAIXELL_ESPERANT) {
        while (i < harbor->n_cua && trobat == 0) {
            if (harbor->cua[i] == vaixell) {
                HARBOR_treureDeLaCua(harbor, i);
                trobat = 1;
            }
            i++;
        }
    }
    vaixell->estat = VAIXELL_CONNECTAT;

    if (tancant_illa == 0) {
        if (vaixell->nom != NULL) {
            asprintf(&missatge, MSG_HA_MARXAT, vaixell->nom, harbor->config->name);
            UTILS_printF(1, missatge);
            free(missatge);
        }
        HARBOR_atracarSeguent(harbor);
    }

    pthread_mutex_unlock(&harbor->mutex_port);
}

void HARBOR_atendreMercat (Vaixell *vaixell, int tipus, char *dades) {
    Harbor *harbor = vaixell->harbor;
    int estat;

    pthread_mutex_lock(&harbor->mutex_port);
    estat = vaixell->estat;
    pthread_mutex_unlock(&harbor->mutex_port);

    if (estat != VAIXELL_ATRACAT) {
        TRAMA_enviarTrama(vaixell->fd, tipus, FLAG_ERROR, TEXT_ERROR_ESTAT);
    } else if (tipus == TIPUS_LIST_MARKET) {
        MARKET_atendreListMarket(vaixell->fd, harbor->mercat);
    } else if (tipus == TIPUS_BUY) {
        MARKET_atendreBuy(vaixell->fd, vaixell->nom, dades, harbor->mercat);
    } else if (tipus == TIPUS_SELL) {
        MARKET_atendreSell(vaixell->fd, vaixell->nom, dades, harbor->mercat);
    } else {
        MARKET_atendreBuyMap(vaixell->fd, vaixell->nom, dades, harbor->config);
    }
}

int HARBOR_errorDeliver (Vaixell *vaixell, int resultat_fitxer, char *motiu) {
    char *missatge = NULL;

    asprintf(&missatge, MSG_DELIVER_ERROR, vaixell->nom);
    UTILS_printF(1, missatge);
    free(missatge);

    if (resultat_fitxer == FITXER_ERROR) {
        return VAIXELL_ACABA;
    }

    TRAMA_enviarTrama(vaixell->fd, TIPUS_DELIVER, FLAG_ERROR, motiu);
    return VAIXELL_CONTINUA;
}

int HARBOR_atendreDeliver (Vaixell *vaixell, char *dades) {
    Harbor *harbor = vaixell->harbor;
    char **trossos = NULL;
    char *path_rebut = NULL;
    char *path_segellat = NULL;
    char *nom_fitxer = NULL;
    char *missatge = NULL;
    int n_trossos = 0;
    int estat;
    int resultat;

    pthread_mutex_lock(&harbor->mutex_port);
    estat = vaixell->estat;
    pthread_mutex_unlock(&harbor->mutex_port);

    trossos = UTILS_splitString(dades, '&', &n_trossos);

    if (estat != VAIXELL_ATRACAT) {
        UTILS_freeTokens(trossos, n_trossos);
        TRAMA_enviarTrama(vaixell->fd, TIPUS_DELIVER, FLAG_ERROR, TEXT_ERROR_ESTAT);
        return VAIXELL_ACABA;
    }

    if (n_trossos != 2 || UTILS_esNumeric(trossos[0]) == TEXT_NO_NUMERIC) {
        UTILS_freeTokens(trossos, n_trossos);
        TRAMA_enviarTrama(vaixell->fd, TIPUS_DELIVER, FLAG_ERROR, TEXT_ERROR_DADES);
        return VAIXELL_ACABA;
    }

    asprintf(&missatge, MSG_DELIVER_PETICIO, vaixell->nom, trossos[0]);
    UTILS_printF(1, missatge);
    free(missatge);
    asprintf(&missatge, MSG_DELIVER_REBENT, trossos[1]);
    UTILS_printF(1, missatge);
    free(missatge);
    UTILS_freeTokens(trossos, n_trossos);

    resultat = FITXER_rebre(vaixell->fd, harbor->config->folder, TEXT_FITXER_DELIVERY, "", &path_rebut);
    if (resultat != FITXER_OK) {
        return HARBOR_errorDeliver(vaixell, resultat, TEXT_ERROR_TRANSFERENCIA);
    }
    UTILS_printF(1, MSG_DELIVER_REBUT);

    asprintf(&missatge, MSG_DELIVER_SEGELLANT, harbor->config->name);
    UTILS_printF(1, missatge);
    free(missatge);

    nom_fitxer = UTILS_nomFitxer(path_rebut);
    if (asprintf(&path_segellat, FORMAT_RUTA_SEGELLADA, harbor->config->folder, nom_fitxer) < 0) {
        free(path_rebut);
        return HARBOR_errorDeliver(vaixell, FITXER_OK, TEXT_ERROR_SERVIDOR);
    }

    if (SPHRAGIS_sign_with_glyph(harbor->config->name, path_rebut, path_segellat) != SPHRAGIS_OK) {
        free(path_rebut);
        free(path_segellat);
        return HARBOR_errorDeliver(vaixell, FITXER_OK, TEXT_ERROR_SEGELL);
    }
    UTILS_printF(1, MSG_DELIVER_SEGELLAT);

    asprintf(&missatge, MSG_DELIVER_ENVIANT, vaixell->nom);
    UTILS_printF(1, missatge);
    free(missatge);

    resultat = FITXER_enviar(vaixell->fd, path_segellat, nom_fitxer, TEXT_FITXER_SIGNED, 1);
    free(path_rebut);
    free(path_segellat);

    if (resultat != FITXER_OK) {
        return HARBOR_errorDeliver(vaixell, resultat, TEXT_ERROR_TRANSFERENCIA);
    }
    UTILS_printF(1, MSG_DELIVER_FET);

    TRAMA_enviarTrama(vaixell->fd, TIPUS_DELIVER, FLAG_OK, TEXT_OK);

    return VAIXELL_CONTINUA;
}

int HARBOR_atendreTrama (Vaixell *vaixell) {
    char *dades = NULL;
    int tipus;
    int flags;
    int continuar = VAIXELL_CONTINUA;

    if (TRAMA_rebreTrama(vaixell->fd, &tipus, &flags, &dades) == TRAMA_ERROR) {
        free(dades);
        return VAIXELL_ACABA;
    }

    if (flags != FLAG_PETICIO) {
        TRAMA_enviarTrama(vaixell->fd, TIPUS_NACK, FLAG_ERROR, TEXT_NACK_FLAGS);
    } else if (tipus == TIPUS_ARRIBADA) {
        HARBOR_atendreArribada(vaixell, dades);
    } else if (tipus == TIPUS_SORTIR_ILLA) {
        continuar = HARBOR_atendreSortida(vaixell);
    } else if (tipus == TIPUS_DELIVER) {
        continuar = HARBOR_atendreDeliver(vaixell, dades);
    } else if (tipus == TIPUS_LIST_MARKET || tipus == TIPUS_BUY || tipus == TIPUS_SELL || tipus == TIPUS_BUY_MAP) {
        HARBOR_atendreMercat(vaixell, tipus, dades);
    } else {
        TRAMA_enviarTrama(vaixell->fd, TIPUS_NACK, FLAG_ERROR, TEXT_NACK_TIPUS);
    }

    free(dades);

    return continuar;
}

void HARBOR_alliberarHarbor (Harbor *harbor) {
    int i;

    for (i = 0; i < harbor->n_vaixells; i++) {
        HARBOR_alliberarVaixell(harbor->vaixells[i]);
    }
    free(harbor->vaixells);
    harbor->vaixells = NULL;
    harbor->n_vaixells = 0;

    free(harbor->cua);
    harbor->cua = NULL;
    harbor->n_cua = 0;

    pthread_mutex_destroy(&harbor->mutex_vaixells);
    pthread_mutex_destroy(&harbor->mutex_port);
}

void HARBOR_mostrarInici (IslandConfig *config, int n_productes) {
    char *missatge = NULL;
    char *text_vaixells = TEXT_VAIXELLS;

    if (config->capacity == 1) {
        text_vaixells = TEXT_VAIXELL;
    }

    asprintf(&missatge, MSG_INICI_ILLA, config->name, config->capacity, text_vaixells, config->n_routes, n_productes);
    UTILS_printF(1, missatge);
    free(missatge);
}

void *HARBOR_atendreVaixell (void *arg) {
    Vaixell *vaixell = (Vaixell *) arg;
    Harbor *harbor = vaixell->harbor;
    fd_set set;
    int max_fd;
    int continuar = VAIXELL_CONTINUA;
    int tancant_illa = 0;

    max_fd = vaixell->fd;
    if (harbor->fd_avis > max_fd) {
        max_fd = harbor->fd_avis;
    }

    while (continuar == VAIXELL_CONTINUA) {
        FD_ZERO(&set);
        FD_SET(vaixell->fd, &set);
        FD_SET(harbor->fd_avis, &set);

        if (select(max_fd + 1, &set, NULL, NULL, NULL) > 0) {
            if (FD_ISSET(harbor->fd_avis, &set)) {
                tancant_illa = 1;
                continuar = VAIXELL_ACABA;
            } else if (FD_ISSET(vaixell->fd, &set)) {
                continuar = HARBOR_atendreTrama(vaixell);
            }
        }
    }

    HARBOR_ferSortir(vaixell, tancant_illa);

    close(vaixell->fd);

    pthread_mutex_lock(&harbor->mutex_vaixells);
    vaixell->acabat = 1;
    pthread_mutex_unlock(&harbor->mutex_vaixells);

    return NULL;
}

void HARBOR_executarServidor (int fd_servidor, Harbor *harbor) {
    fd_set set;
    int max_fd;
    int sortir = 0;

    if (fd_servidor >= FD_SETSIZE || harbor->fd_avis >= FD_SETSIZE) {
        UTILS_printF(1, MSG_ERROR_DESCRIPTOR);
        return;
    }

    max_fd = fd_servidor;
    if (harbor->fd_avis > max_fd) {
        max_fd = harbor->fd_avis;
    }

    while (sortir == 0) {
        FD_ZERO(&set);
        FD_SET(fd_servidor, &set);
        FD_SET(harbor->fd_avis, &set);

        if (select(max_fd + 1, &set, NULL, NULL, NULL) > 0) {
            if (FD_ISSET(harbor->fd_avis, &set)) {
                sortir = 1;
            } else if (FD_ISSET(fd_servidor, &set)) {
                HARBOR_acceptarVaixell(harbor, fd_servidor);
            }
        }

        HARBOR_netejarVaixellsAcabats(harbor);
    }

    HARBOR_esperarVaixells(harbor);
}
