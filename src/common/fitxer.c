#include "fitxer.h"

int FITXER_midaFitxer (char *path) {
    int fd;
    int mida;

    fd = open(path, O_RDONLY);

    if (fd < 0) {
        UTILS_printF(1, MSG_FITXER_ERROR_OBRIR);
        return FITXER_ERROR;
    }

    mida = lseek(fd, 0, SEEK_END);

    if (mida < 0) {
        UTILS_printF(1, MSG_FITXER_ERROR_MIDA);
    }

    close(fd);

    return mida;
}

int FITXER_calcularMD5 (char *path, char *md5) {
    int fd_pipe[2];
    int pid, estat_fill, resultat_wait;
    int llegits = 0;
    int resultat_read = 1;
    int resultat = FITXER_ERROR;
    char lletra;

    md5[0] = '\0';

    if (pipe(fd_pipe) < 0) {
        UTILS_printF(1, MSG_FITXER_ERROR_MD5);
        return FITXER_ERROR;
    }

    pid = fork();
    if (pid < 0) {
        close(fd_pipe[0]);
        close(fd_pipe[1]);
        UTILS_printF(1, MSG_FITXER_ERROR_MD5);
        return FITXER_ERROR;
    }

    if (pid == 0) {
        close(fd_pipe[0]);
        if (dup2(fd_pipe[1], 1) < 0) {
            exit(1);
        }
        close(fd_pipe[1]);
        execlp(COMANDA_MD5, COMANDA_MD5, path, NULL);
        exit(1);
    }

    close(fd_pipe[1]);

    while (resultat_read > 0) {
        resultat_read = read(fd_pipe[0], &lletra, 1);
        if (resultat_read > 0 && llegits < MIDA_MD5) {
            md5[llegits] = lletra;
            llegits++;
        }
    }

    close(fd_pipe[0]);
    resultat_wait = waitpid(pid, &estat_fill, 0);

    if (resultat_wait == pid && estat_fill == 0 && resultat_read == 0 && llegits == MIDA_MD5) {
        md5[MIDA_MD5] = '\0';
        resultat = FITXER_OK;
    } else {
        md5[0] = '\0';
        UTILS_printF(1, MSG_FITXER_ERROR_MD5);
    }

    return resultat;
}