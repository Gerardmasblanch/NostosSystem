/***********************************************
*
* @Proposit: Implementacio de l'enviament i la recepcio de trames i de
*            l'obertura de sockets.
* @Autor/s: Arnau Ricart i Gerard Mas
* @Data creacio: 29/10/2026
* @Data ultima modificacio: 29/10/2026
*
************************************************/

//Include del fitxer .h del mateix modul
#include "trama.h"

int enviarTrama (int fd, int tipus, int flags, char *dades) {
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

int rebreTrama (int fd, int *tipus, int *flags, char **dades) {
    unsigned char trama[MIDA_TRAMA];
    int llegits;
    int mida_dades;
    int resultat = TRAMA_ERROR;

    *dades = NULL;

    llegits = read(fd, trama, MIDA_TRAMA);
    if (llegits == MIDA_TRAMA) {
        mida_dades = trama[POS_MIDA];
        if (mida_dades > MIDA_DADES) {
            mida_dades = MIDA_DADES;
        }

        *dades = malloc(sizeof(char) * (mida_dades + 1));
        if (*dades == NULL) {
            printF(1, MSG_SENSE_MEMORIA);
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



int obrirServidor (char *ip, int port) {
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



int connectarServidor (char *ip, int port) {
    struct sockaddr_in s_addr;
    int fd_socket = ERR_CODI;

    if (esPortValid(port) == PORT_NO_VALID) {
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

