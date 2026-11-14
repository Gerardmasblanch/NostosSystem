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

void HARBOR_inicialitzarHarbor (Harbor *harbor) {
    harbor->vaixells = NULL;
    harbor->n_vaixells = 0;
    harbor->cua = NULL;
    harbor->n_cua = 0;
    harbor->n_atracats = 0;
}

int HARBOR_afegirVaixell (Harbor *harbor, int fd) {
    Vaixell *nou_vaixell = NULL;

    nou_vaixell = realloc(harbor->vaixells, sizeof(Vaixell) * (harbor->n_vaixells + 1));

    if (nou_vaixell == NULL) {
        UTILS_printF(1, MSG_SENSE_MEMORIA);
        close(fd);
        return ERR_CODI; 
    }

    harbor->vaixells = nou_vaixell;
    harbor->vaixells[harbor->n_vaixells].fd = fd;
    harbor->vaixells[harbor->n_vaixells].nom = NULL; 
    harbor->vaixells[harbor->n_vaixells].estat = VAIXELL_CONNECTAT; 
    harbor->n_vaixells++;
    return 0;
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

int HARBOR_prepararSet (fd_set *set, int fd_servidor, Harbor *harbor) {
    int i;
    int max_fd = fd_servidor;

    FD_ZERO(set);
    FD_SET(fd_servidor, set);

    for (i = 0; i < harbor->n_vaixells; i++) {
        FD_SET(harbor->vaixells[i].fd, set);
        if (harbor->vaixells[i].fd > max_fd) {
            max_fd = harbor->vaixells[i].fd;
        }
    }

    return max_fd;
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

void HARBOR_atendreVaixells (Harbor *harbor, fd_set *set, IslandConfig *config, Mercat *mercat) {
    int i;

    for (i = harbor->n_vaixells - 1; i >= 0; i--) {
        if (FD_ISSET(harbor->vaixells[i].fd, set)) {
            HARBOR_atendreTrama(harbor, i, config, mercat);
        }
    }
}

void HARBOR_alliberarHarbor (Harbor *harbor) {
    while (harbor->n_vaixells > 0) {
        HARBOR_treureVaixell(harbor, harbor->n_vaixells - 1);
    }

    free(harbor->vaixells);
    free(harbor->cua);
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

void HARBOR_executarServidor (int fd_servidor, Harbor *harbor, IslandConfig *config, Mercat *mercat, int *sortir) {
    fd_set set;
    int max_fd;
    int fd_vaixell;

    while (*sortir == 0) {
        max_fd = HARBOR_prepararSet(&set, fd_servidor, harbor);

        if (select(max_fd + 1, &set, NULL, NULL, NULL) > 0) {
            if (FD_ISSET(fd_servidor, &set)) {
                fd_vaixell = accept(fd_servidor, NULL, NULL);
                if (fd_vaixell != ERR_CODI) {
                    HARBOR_afegirVaixell(harbor, fd_vaixell);
                }
            }
            HARBOR_atendreVaixells(harbor, &set, config, mercat);
        }
    }
}