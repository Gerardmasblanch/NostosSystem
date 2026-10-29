/***********************************************
*
* @Proposit: Implementacio del servidor de l'illa: connexions, port i cua
*            d'espera.
* @Autor/s: Arnau Ricart i Gerard Mas
* @Data creacio: 29/10/2026
* @Data ultima modificacio: 29/10/2026
*
************************************************/

//include del fitxer .h del mateix mòdul
#include "harbor.h"


//funcions i procediments

void inicialitzarHarbor (Harbor *harbor) {
    harbor->vaixells = NULL;
    harbor->n_vaixells = 0;
    harbor->cua = NULL;
    harbor->n_cua = 0;
    harbor->n_atracats = 0;
}

int afegirVaixell (Harbor *harbor, int fd) {
    Vaixell *nou_vaixell = NULL;

    nou_vaixell = realloc(harbor->vaixells, sizeof(Vaixell) * (harbor->n_vaixells + 1));

    if (nou_vaixell == NULL) {
        printF(1, MSG_SENSE_MEMORIA);
        close(fd);
        return ERR_CODI; // Error de memoria
    }

    harbor->vaixells = nou_vaixell;
    harbor->vaixells[harbor->n_vaixells].fd = fd;
    harbor->vaixells[harbor->n_vaixells].nom = NULL; // Inicialitzar el nom a NULL
    harbor->vaixells[harbor->n_vaixells].estat = VAIXELL_CONNECTAT; // Estat inicial
    harbor->n_vaixells++;
    return 0; // Vaixell afegit correctament
}

void treureDeLaCua (Harbor *harbor, int posicio) {
    int i = 0;

    for (i = posicio; i < harbor->n_cua - 1; i++) {
        harbor->cua[i] = harbor->cua[i + 1];
    }
    harbor->n_cua--;
}

void treureVaixell (Harbor *harbor, int index) {
    int i;

    if (harbor->vaixells[index].estat == VAIXELL_ATRACAT) {
        harbor->n_atracats--;
    } else if (harbor->vaixells[index].estat == VAIXELL_ESPERANT) {
        for (i = 0; i < harbor->n_cua; i++) {
            if (harbor->cua[i] == harbor->vaixells[index].fd) {
                treureDeLaCua(harbor, i);
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


int buscarVaixell (Harbor *harbor, int fd) {
    int i;
    int index = ERR_CODI;

    for (i = 0; i < harbor->n_vaixells; i++) {
        if (harbor->vaixells[i].fd == fd) {
            index = i;
        }
    }

    return index;
}

void atracarSeguent (Harbor *harbor, int capacitat) {
    char *missatge = NULL;
    int fd;
    int index;

    while (harbor->n_atracats < capacitat && harbor->n_cua > 0) {
        fd = harbor->cua[0];
        treureDeLaCua(harbor, 0);

        index = buscarVaixell(harbor, fd);
        if (index != ERR_CODI) {
            harbor->vaixells[index].estat = VAIXELL_ATRACAT;
            harbor->n_atracats++;
            enviarTrama(fd, TIPUS_ARRIBADA, FLAG_OK, TEXT_DOCKED);

            asprintf(&missatge, MSG_DOCKED_ENVIAT, harbor->vaixells[index].nom);
            printF(1, missatge);
            free(missatge);
        }
    }
}

int afegirALaCua (Harbor *harbor, int fd) {
    int *nova_cua = NULL;

    nova_cua = realloc(harbor->cua, sizeof(int) * (harbor->n_cua + 1));
    if (nova_cua == NULL) {
        printF(1, MSG_SENSE_MEMORIA);
        return ERR_CODI;
    }

    harbor->cua = nova_cua;
    harbor->cua[harbor->n_cua] = fd;
    harbor->n_cua++;

    return 0;
}

void atendreArribada (Harbor *harbor, int index, char *dades, IslandConfig *config) {
    Vaixell *vaixell = NULL;
    char *missatge = NULL;

    vaixell = &harbor->vaixells[index];

    if (vaixell->estat != VAIXELL_CONNECTAT) {
        enviarTrama(vaixell->fd, TIPUS_ARRIBADA, FLAG_ERROR, TEXT_ERROR_ESTAT);
        return;
    }

    if (strlen(dades) == 0) {
        enviarTrama(vaixell->fd, TIPUS_ARRIBADA, FLAG_ERROR, TEXT_ERROR_DADES);
        return;
    }

    vaixell->nom = malloc(sizeof(char) * (strlen(dades) + 1));
    if (vaixell->nom == NULL) {
        printF(1, MSG_SENSE_MEMORIA);
        enviarTrama(vaixell->fd, TIPUS_ARRIBADA, FLAG_ERROR, TEXT_ERROR_SERVIDOR);
        return;
    }
    strcpy(vaixell->nom, dades);

    asprintf(&missatge, MSG_APROXIMANT, vaixell->nom, config->name);
    printF(1, missatge);
    free(missatge);

    if (harbor->n_atracats < config->capacity) {

        vaixell->estat = VAIXELL_ATRACAT;
        harbor->n_atracats++;
        enviarTrama(vaixell->fd, TIPUS_ARRIBADA, FLAG_OK, TEXT_DOCKED);

        asprintf(&missatge, MSG_ATRACAT, vaixell->nom);
        printF(1, missatge);
        free(missatge);
    } else if (afegirALaCua(harbor, vaixell->fd) == 0) {

        vaixell->estat = VAIXELL_ESPERANT;
        enviarTrama(vaixell->fd, TIPUS_ARRIBADA, FLAG_OK, TEXT_WAIT);

        asprintf(&missatge, MSG_PORT_PLE, vaixell->nom);
        printF(1, missatge);
        free(missatge);
    } else {
        enviarTrama(vaixell->fd, TIPUS_ARRIBADA, FLAG_ERROR, TEXT_ERROR_SERVIDOR);
    }
}

void ferSortir (Harbor *harbor, int index, IslandConfig *config) {
    char *missatge = NULL;

    if (harbor->vaixells[index].nom != NULL) {
        asprintf(&missatge, MSG_HA_MARXAT, harbor->vaixells[index].nom);
        printF(1, missatge);
        free(missatge);

    }

    treureVaixell(harbor, index);
    atracarSeguent(harbor, config->capacity);
}

void atendreSortida (Harbor *harbor, int index, IslandConfig *config) {

    if (harbor->vaixells[index].estat == VAIXELL_CONNECTAT) {

        enviarTrama(harbor->vaixells[index].fd, TIPUS_SORTIR_ILLA, FLAG_ERROR, TEXT_ERROR_ESTAT);
    } else {
        enviarTrama(harbor->vaixells[index].fd, TIPUS_SORTIR_ILLA, FLAG_OK, TEXT_OK);
        ferSortir(harbor, index, config);
    }
}

int prepararSet (fd_set *set, int fd_servidor, Harbor *harbor) {
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

void atendreTrama (Harbor *harbor, int index, IslandConfig *config) {
    char *dades = NULL;
    int fd;
    int tipus;
    int flags;

    fd = harbor->vaixells[index].fd;

    if (rebreTrama(fd, &tipus, &flags, &dades) == TRAMA_ERROR) {

        ferSortir(harbor, index, config);
    } else if (flags != FLAG_PETICIO) {
        enviarTrama(fd, TIPUS_NACK, FLAG_ERROR, TEXT_NACK_FLAGS);
    } else if (tipus == TIPUS_ARRIBADA) {
        atendreArribada(harbor, index, dades, config);
    } else if (tipus == TIPUS_SORTIR_ILLA) {
        atendreSortida(harbor, index, config);
    } else {
        enviarTrama(fd, TIPUS_NACK, FLAG_ERROR, TEXT_NACK_TIPUS);
    }

    free(dades);
}

void atendreVaixells (Harbor *harbor, fd_set *set, IslandConfig *config) {
    int i;

    for (i = harbor->n_vaixells - 1; i >= 0; i--) {
        if (FD_ISSET(harbor->vaixells[i].fd, set)) {
            atendreTrama(harbor, i, config);
        }
    }
}

void alliberarHarbor (Harbor *harbor) {
    while (harbor->n_vaixells > 0) {
        treureVaixell(harbor, harbor->n_vaixells - 1);
    }

    free(harbor->vaixells);
    free(harbor->cua);
}

void mostrarInici (IslandConfig *config, int n_productes) {
    char *missatge = NULL;
    char *text_vaixells = TEXT_VAIXELLS;

    if (config->capacity == 1) {
        text_vaixells = TEXT_VAIXELL;
    }

    asprintf(&missatge, MSG_INICI_ILLA, config->name, config->capacity, text_vaixells, config->n_routes, n_productes);
    printF(1, missatge);
    free(missatge);
}

void executarServidor (int fd_servidor, Harbor *harbor, IslandConfig *config, int *sortir) {
    fd_set set;
    int max_fd;
    int fd_vaixell;

    while (*sortir == 0) {
        max_fd = prepararSet(&set, fd_servidor, harbor);

        if (select(max_fd + 1, &set, NULL, NULL, NULL) > 0) {
            if (FD_ISSET(fd_servidor, &set)) {
                fd_vaixell = accept(fd_servidor, NULL, NULL);
                if (fd_vaixell != ERR_CODI) {
                    afegirVaixell(harbor, fd_vaixell);
                }
            }
            atendreVaixells(harbor, &set, config);
        }
    }
}