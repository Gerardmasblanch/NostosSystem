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