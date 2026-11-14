/***********************************************
*
* @Proposit: Implementacio del servidor de l'illa, connexions, port i cua
*            d'espera.
* @Autor/s: Arnau Ricart i Gerard Mas
* @Data creacio: 29/10/2026
* @Data ultima modificacio: 29/10/2026
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

void HARBOR_treureDeLaCua (Harbor *harbor, int posicio) {
    int i = 0;

    for (i = posicio; i < harbor->n_cua - 1; i++) {
        harbor->cua[i] = harbor->cua[i + 1];
    }
    harbor->n_cua--;
}

void HARBOR_treureVaixell (Harbor *harbor, int index) {
    int i;

    if (harbor->vaixells[index].estat == VAIXELL_ATRACAT) {
        harbor->n_atracats--;
    } else if (harbor->vaixells[index].estat == VAIXELL_ESPERANT) {
        for (i = 0; i < harbor->n_cua; i++) {
            if (harbor->cua[i] == harbor->vaixells[index].fd) {
                HARBOR_treureDeLaCua(harbor, i);
            }
        }
    }

    close(harbor->vaixells[index].fd);
    free(harbor->vaixells[index].nom);

    for (i = index; i < harbor->n_vaixells - 1; i++) {
        harbor->vaixells[i] = harbor->vaixells[i + 1];
    }
    harbor->n_vaixells--;
}


int HARBOR_buscarVaixell (Harbor *harbor, int fd) {
    int i;
    int index = ERR_CODI;

    for (i = 0; i < harbor->n_vaixells; i++) {
        if (harbor->vaixells[i].fd == fd) {
            index = i;
        }
    }

    return index;
}

void HARBOR_atracarSeguent (Harbor *harbor, int capacitat) {
    char *missatge = NULL;
    int fd;
    int index;

    while (harbor->n_atracats < capacitat && harbor->n_cua > 0) {
        fd = harbor->cua[0];
        HARBOR_treureDeLaCua(harbor, 0);

        index = HARBOR_buscarVaixell(harbor, fd);
        if (index != ERR_CODI) {
            harbor->vaixells[index].estat = VAIXELL_ATRACAT;
            harbor->n_atracats++;
            TRAMA_enviarTrama(fd, TIPUS_ARRIBADA, FLAG_OK, TEXT_DOCKED);

            asprintf(&missatge, MSG_DOCKED_ENVIAT, harbor->vaixells[index].nom);
            UTILS_printF(1, missatge);
            free(missatge);
        }
    }
}

int HARBOR_afegirALaCua (Harbor *harbor, int fd) {
    int *nova_cua = NULL;

    nova_cua = realloc(harbor->cua, sizeof(int) * (harbor->n_cua + 1));
    if (nova_cua == NULL) {
        UTILS_printF(1, MSG_SENSE_MEMORIA);
        return ERR_CODI;
    }

    harbor->cua = nova_cua;
    harbor->cua[harbor->n_cua] = fd;
    harbor->n_cua++;

    return 0;
}

void HARBOR_atendreArribada (Harbor *harbor, int index, char *dades, IslandConfig *config) {
    Vaixell *vaixell = NULL;
    char *missatge = NULL;

    vaixell = &harbor->vaixells[index];

    if (vaixell->estat != VAIXELL_CONNECTAT) {
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

    asprintf(&missatge, MSG_APROXIMANT, vaixell->nom, config->name);
    UTILS_printF(1, missatge);
    free(missatge);

    if (harbor->n_atracats < config->capacity) {

        vaixell->estat = VAIXELL_ATRACAT;
        harbor->n_atracats++;
        TRAMA_enviarTrama(vaixell->fd, TIPUS_ARRIBADA, FLAG_OK, TEXT_DOCKED);

        asprintf(&missatge, MSG_ATRACAT, vaixell->nom);
        UTILS_printF(1, missatge);
        free(missatge);
    } else if (HARBOR_afegirALaCua(harbor, vaixell->fd) == 0) {

        vaixell->estat = VAIXELL_ESPERANT;
        TRAMA_enviarTrama(vaixell->fd, TIPUS_ARRIBADA, FLAG_OK, TEXT_WAIT);

        asprintf(&missatge, MSG_PORT_PLE, vaixell->nom);
        UTILS_printF(1, missatge);
        free(missatge);
    } else {
        TRAMA_enviarTrama(vaixell->fd, TIPUS_ARRIBADA, FLAG_ERROR, TEXT_ERROR_SERVIDOR);
    }
}

void HARBOR_ferSortir (Harbor *harbor, int index, IslandConfig *config) {
    char *missatge = NULL;

    if (harbor->vaixells[index].nom != NULL) {
        asprintf(&missatge, MSG_HA_MARXAT, harbor->vaixells[index].nom);
        UTILS_printF(1, missatge);
        free(missatge);

    }

    HARBOR_treureVaixell(harbor, index);
    HARBOR_atracarSeguent(harbor, config->capacity);
}

void HARBOR_atendreSortida (Harbor *harbor, int index, IslandConfig *config) {

    if (harbor->vaixells[index].estat == VAIXELL_CONNECTAT) {

        TRAMA_enviarTrama(harbor->vaixells[index].fd, TIPUS_SORTIR_ILLA, FLAG_ERROR, TEXT_ERROR_ESTAT);
    } else {
        TRAMA_enviarTrama(harbor->vaixells[index].fd, TIPUS_SORTIR_ILLA, FLAG_OK, TEXT_OK);
        HARBOR_ferSortir(harbor, index, config);
    }
}

void HARBOR_atendreMercat (Harbor *harbor, int index, int tipus, char *dades, IslandConfig *config, Mercat *mercat) {
    Vaixell *vaixell = NULL;

    vaixell = &harbor->vaixells[index];

    if (vaixell->estat != VAIXELL_ATRACAT) {
        TRAMA_enviarTrama(vaixell->fd, tipus, FLAG_ERROR, TEXT_ERROR_ESTAT);
    } else if (tipus == TIPUS_LIST_MARKET) {
        MARKET_atendreListMarket(vaixell->fd, mercat);
    } else if (tipus == TIPUS_BUY) {
        MARKET_atendreBuy(vaixell->fd, vaixell->nom, dades, mercat);
    } else if (tipus == TIPUS_SELL) {
        MARKET_atendreSell(vaixell->fd, vaixell->nom, dades, mercat);
    } else {
        MARKET_atendreBuyMap(vaixell->fd, vaixell->nom, dades, config);
    }
}

void HARBOR_atendreTrama (Harbor *harbor, int index, IslandConfig *config, Mercat *mercat) {
    char *dades = NULL;
    int fd;
    int tipus;
    int flags;

    fd = harbor->vaixells[index].fd;

    if (TRAMA_rebreTrama(fd, &tipus, &flags, &dades) == TRAMA_ERROR) {

        HARBOR_ferSortir(harbor, index, config);
    } else if (flags != FLAG_PETICIO) {
        TRAMA_enviarTrama(fd, TIPUS_NACK, FLAG_ERROR, TEXT_NACK_FLAGS);
    } else if (tipus == TIPUS_ARRIBADA) {
        HARBOR_atendreArribada(harbor, index, dades, config);
    } else if (tipus == TIPUS_SORTIR_ILLA) {
        HARBOR_atendreSortida(harbor, index, config);
    } else if (tipus == TIPUS_LIST_MARKET || tipus == TIPUS_BUY || tipus == TIPUS_SELL || tipus == TIPUS_BUY_MAP) {
        HARBOR_atendreMercat(harbor, index, tipus, dades, config, mercat);

    } else {
        TRAMA_enviarTrama(fd, TIPUS_NACK, FLAG_ERROR, TEXT_NACK_TIPUS);
    }

    free(dades);
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