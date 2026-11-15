/***********************************************
*
* @Proposit: Implementacio de l'enviament i la recepcio de trames i de
*            l'obertura de sockets.
* @Autor/s: Arnau Ricart i Gerard Mas
* @Data creacio: 29/10/2026
* @Data ultima modificacio: 15/11/2026
*
************************************************/

//Include del fitxer .h del mateix modul
#include "trama.h"

//Variables del modul: extrem de lectura de la pipe d'avis, nomes s'escriu un cop al main
static int fd_avis = ERR_CODI;

void TRAMA_establirAvis (int fd) {
    fd_avis = fd;
}

int TRAMA_esperarDades (int fd) {
    fd_set set;
    int max_fd;
    int final = 0;
    int resultat = TRAMA_ERROR;

    while (final == 0) {
        FD_ZERO(&set);
        FD_SET(fd, &set);
        max_fd = fd;
        if (fd_avis != ERR_CODI) {
            FD_SET(fd_avis, &set);
            if (fd_avis > max_fd) {
                max_fd = fd_avis;
            }
        }

        if (select(max_fd + 1, &set, NULL, NULL, NULL) > 0) {
            final = 1;
            if (fd_avis == ERR_CODI || FD_ISSET(fd_avis, &set) == 0) {
                resultat = TRAMA_OK;
            }
        }
    }

    return resultat;
}

int TRAMA_enviarTrama (int fd, int tipus, int flags, char *dades) {
    unsigned char trama[MIDA_TRAMA];
    int mida_dades;
    int escrits;
    int resultat = TRAMA_ERROR;

    memset(trama, 0, MIDA_TRAMA);

    mida_dades = strlen(dades);
    if (mida_dades > MIDA_DADES) {
        mida_dades = MIDA_DADES;
    }

    trama[POS_TIPUS] = tipus;
    trama[POS_FLAGS] = flags;
    trama[POS_MIDA] = mida_dades;
    memcpy(&trama[POS_DADES], dades, mida_dades);

    escrits = write(fd, trama, MIDA_TRAMA);
    if (escrits == MIDA_TRAMA) {
        resultat = TRAMA_OK;
    }

    return resultat;
}

int TRAMA_enviarTramaBinaria (int fd, int tipus, int flags, char *dades, int mida) {
    unsigned char trama[MIDA_TRAMA];
    int escrits;
    int resultat = TRAMA_ERROR;

    if (mida < 0 || mida > MIDA_DADES) {
        return TRAMA_ERROR;
    }

    if (mida > 0 && dades == NULL) {
        return TRAMA_ERROR;
    }

    memset(trama, 0, MIDA_TRAMA);

    trama[POS_TIPUS] = tipus;
    trama[POS_FLAGS] = flags;
    trama[POS_MIDA] = mida;

    if (mida > 0) {
        memcpy(&trama[POS_DADES], dades, mida);
    }

    escrits = write(fd, trama, MIDA_TRAMA);

    if (escrits == MIDA_TRAMA) {
        resultat = TRAMA_OK;
    }

    return resultat;
}

int TRAMA_rebreTrama (int fd, int *tipus, int *flags, char **dades) {
    unsigned char trama[MIDA_TRAMA];
    int llegits = 0;
    int resultat_read = 1;
    int mida_dades;
    int resultat = TRAMA_ERROR;

    *dades = NULL;

    while (llegits < MIDA_TRAMA && resultat_read > 0) {
        resultat_read = ERR_CODI;
        if (TRAMA_esperarDades(fd) == TRAMA_OK) {
            resultat_read = read(fd, &trama[llegits], MIDA_TRAMA - llegits);
        }
        if (resultat_read > 0) {
            llegits = llegits + resultat_read;
        }
    }

    if (llegits == MIDA_TRAMA) {
        mida_dades = trama[POS_MIDA];
        if (mida_dades > MIDA_DADES) {
            mida_dades = MIDA_DADES;
        }

        *dades = malloc(sizeof(char) * (mida_dades + 1));
        if (*dades == NULL) {
            UTILS_printF(1, MSG_SENSE_MEMORIA);
        } else {
            memcpy(*dades, &trama[POS_DADES], mida_dades);
            (*dades)[mida_dades] = '\0';
            *tipus = trama[POS_TIPUS];
            *flags = trama[POS_FLAGS];
            resultat = TRAMA_OK;
        }
    }

    return resultat;
}

int TRAMA_rebreTramaBinaria (int fd, int *tipus, int *flags, char **dades, int *mida) {
    unsigned char trama[MIDA_TRAMA];
    int llegits = 0;
    int resultat_read = 1;
    int mida_dades;

    *dades = NULL;
    *mida = 0;

    while (llegits < MIDA_TRAMA && resultat_read > 0) {
        resultat_read = ERR_CODI;
        if (TRAMA_esperarDades(fd) == TRAMA_OK) {
            resultat_read = read(fd, &trama[llegits], MIDA_TRAMA - llegits);
        }

        if (resultat_read > 0) {
            llegits = llegits + resultat_read;
        }
    }

    if (llegits != MIDA_TRAMA) {
        return TRAMA_ERROR;
    }

    mida_dades = trama[POS_MIDA];

    if (mida_dades > MIDA_DADES) {
        if (TRAMA_enviarTrama(fd, TIPUS_NACK, FLAG_ERROR, TEXT_TRAMA_INVALID_LENGTH) != TRAMA_OK) {
            UTILS_printF(1, MSG_TRAMA_ERROR_NACK);
        }
        return TRAMA_ERROR;
    }

    *dades = malloc(sizeof(char) * (mida_dades + 1));

    if (*dades == NULL) {
        UTILS_printF(1, MSG_SENSE_MEMORIA);
        return TRAMA_ERROR;
    }

    memcpy(*dades, &trama[POS_DADES], mida_dades);
    (*dades)[mida_dades] = '\0';

    *tipus = trama[POS_TIPUS];
    *flags = trama[POS_FLAGS];
    *mida = mida_dades;

    return TRAMA_OK;
}

int TRAMA_obrirServidor (char *ip, int port) {
    struct sockaddr_in s_addr;
    int fd_socket = ERR_CODI;

    fd_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd_socket == ERR_CODI) {
        return ERR_CODI;
    }

    memset(&s_addr, 0, sizeof(s_addr));
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &s_addr.sin_addr) <= 0) {
        close(fd_socket);
        return ERR_CODI;
    }

    if (bind(fd_socket, (void *) &s_addr, sizeof(s_addr)) < 0) {
        close(fd_socket);
        return ERR_CODI;
    }

    if (listen(fd_socket, MAX_CONNEXIONS) < 0) {
        close(fd_socket);
        return ERR_CODI;
    }

    return fd_socket;
}

int TRAMA_connectarServidor (char *ip, int port) {
    struct sockaddr_in s_addr;
    int fd_socket = ERR_CODI;

    if (UTILS_esPortValid(port) == PORT_NO_VALID) {
        return ERR_CODI;
    }

    fd_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd_socket == ERR_CODI) {
        return ERR_CODI;
    }

    memset(&s_addr, 0, sizeof(s_addr));
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &s_addr.sin_addr) <= 0) {
        close(fd_socket);
        return ERR_CODI;
    }

    if (connect(fd_socket, (struct sockaddr *) &s_addr, sizeof(s_addr)) < 0) {
        close(fd_socket);
        return ERR_CODI;
    }

    return fd_socket;
}

