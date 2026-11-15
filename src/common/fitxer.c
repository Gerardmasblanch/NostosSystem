/***********************************************
*
* @Proposit: Implementacio de les operacions amb fitxers: mida, MD5 i enviament i recepcio de fitxers per trames.
* @Autor/s: Arnau Ricart i Gerard Mas
* @Data creacio: 14/11/2026
* @Data ultima modificacio: 15/11/2026
*
************************************************/

//Include del fitxer .h del mateix modul
#include "fitxer.h"

int FITXER_midaFitxer (char *path) {
    char bloc[MIDA_DADES];
    int fd;
    int mida = 0;
    int llegits = 1;

    fd = open(path, O_RDONLY);
    if (fd < 0) {
        UTILS_printF(1, MSG_FITXER_ERROR_OBRIR);
        return FITXER_ERROR;
    }

    while (llegits > 0) {
        llegits = read(fd, bloc, MIDA_DADES);
        if (llegits > 0) {
            mida = mida + llegits;
        }
    }

    close(fd);

    if (llegits < 0) {
        UTILS_printF(1, MSG_FITXER_ERROR_MIDA);
        return FITXER_ERROR;
    }

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

int FITXER_enviarCapcalera (int fd, char *nom_fitxer, char *etiqueta, int mida, char *md5, int es_resposta) {
    char *dades = NULL;
    int flags = FLAG_PETICIO;
    int resultat = FITXER_ERROR;

    if (asprintf(&dades, FORMAT_FITXER_CAPCALERA, etiqueta, nom_fitxer, mida, md5) < 0) {
        UTILS_printF(1, MSG_SENSE_MEMORIA);
        return FITXER_ERROR;
    }

    if (strlen(dades) > MIDA_DADES) {
        UTILS_printF(1, MSG_FITXER_CAPCALERA_LLARGA);
        free(dades);
        return FITXER_ERROR;
    }

    if (es_resposta == 1) {
        flags = FLAG_OK;
    }

    if (TRAMA_enviarTrama(fd, TIPUS_FITXER_CAPCALERA, flags, dades) == TRAMA_OK) {
        resultat = FITXER_OK;
    } else {
        UTILS_printF(1, MSG_FITXER_ERROR_ENVIAR);
    }

    free(dades);

    return resultat;
}

int FITXER_esperarReady (int fd) {
    int tipus;
    int flags;
    int mida;
    int mida_ready = strlen(TEXT_FITXER_READY);
    char *dades = NULL;
    int resultat = FITXER_ERROR;
    char *error = NULL;

    if (TRAMA_rebreTramaBinaria(fd, &tipus, &flags, &dades, &mida) != TRAMA_OK) {
        UTILS_printF(1, MSG_FITXER_ERROR_REBRE);
        return FITXER_ERROR;
    }

    if (tipus == TIPUS_FITXER_READY && flags == FLAG_OK && mida == mida_ready && strcmp(dades, TEXT_FITXER_READY) == 0) {
        resultat = FITXER_OK;
    } else if (tipus == TIPUS_FITXER_READY && flags == FLAG_ERROR) {
        UTILS_printF(1, MSG_FITXER_REBUTJAT);
        resultat = FITXER_REBUTJAT;
    } else if (tipus == TIPUS_NACK) {
        UTILS_printF(1, MSG_FITXER_RESPOSTA_INVALIDA);
    } else if (tipus != TIPUS_FITXER_READY) {
        error = TEXT_FITXER_INVALID_TYPE;
    } else if (flags != FLAG_OK) {
        error = TEXT_FITXER_INVALID_FLAGS;
    } else {
        error = TEXT_FITXER_INVALID_DATA;
    }

    free(dades);

    if (error != NULL) {
        FITXER_enviarNack(fd, error);
    }

    return resultat;
}

int FITXER_enviarBlocs (int fd, int fd_fitxer, int mida, int es_resposta) {
    char bloc[MIDA_DADES];
    int flags = FLAG_BINARI;
    int enviats = 0;
    int mida_bloc;
    int llegits_bloc;
    int resultat_read;

    if (mida < 0) {
        UTILS_printF(1, MSG_FITXER_MIDA_INVALIDA);
        return FITXER_ERROR;
    }

    if (es_resposta == 1) {
        flags = FLAG_BINARI_RESPOSTA;
    }

    while (enviats < mida) {
        mida_bloc = mida - enviats;
        if (mida_bloc > MIDA_DADES) {
            mida_bloc = MIDA_DADES;
        }

        llegits_bloc = 0;
        while (llegits_bloc < mida_bloc) {
            resultat_read = read(fd_fitxer, &bloc[llegits_bloc], mida_bloc - llegits_bloc);

            if (resultat_read <= 0) {
                UTILS_printF(1, MSG_FITXER_ERROR_LLEGIR);
                return FITXER_ERROR;
            }

            llegits_bloc = llegits_bloc + resultat_read;
        }

        if (TRAMA_enviarTramaBinaria(fd, TIPUS_FITXER_DADES, flags, bloc, mida_bloc) != TRAMA_OK) {
            UTILS_printF(1, MSG_FITXER_ERROR_ENVIAR);
            return FITXER_ERROR;
        }

        enviats = enviats + mida_bloc;
    }

    return FITXER_OK;
}

int FITXER_esperarAck (int fd) {
    int tipus;
    int flags;
    int mida;
    int mida_text;
    char *dades = NULL;
    int resultat = FITXER_ERROR;
    char *error = NULL;

    if (TRAMA_rebreTramaBinaria(fd, &tipus, &flags, &dades, &mida) != TRAMA_OK) {
        UTILS_printF(1, MSG_FITXER_ERROR_REBRE);
        return FITXER_ERROR;
    }

    mida_text = strlen(dades);

    if (tipus == TIPUS_NACK) {
        UTILS_printF(1, MSG_FITXER_RESPOSTA_INVALIDA);
    } else if (tipus != TIPUS_FITXER_ACK) {
        error = TEXT_FITXER_INVALID_TYPE;
    } else if (flags != FLAG_OK && flags != FLAG_ERROR) {
        error = TEXT_FITXER_INVALID_FLAGS;
    } else if (mida != mida_text) {
        error = TEXT_FITXER_INVALID_DATA;
    } else if (flags == FLAG_OK && strcmp(dades, TEXT_FITXER_OK) == 0) {
        resultat = FITXER_OK;
    } else if (flags == FLAG_ERROR && strcmp(dades, TEXT_FITXER_ERROR_MIDA) == 0) {
        UTILS_printF(1, MSG_FITXER_MIDA_DIFERENT);
        resultat = FITXER_ERROR_MIDA;
    } else if (flags == FLAG_ERROR && strcmp(dades, TEXT_FITXER_ERROR_MD5) == 0) {
        UTILS_printF(1, MSG_FITXER_MD5_DIFERENT);
        resultat = FITXER_ERROR_MD5;
    } else {
        error = TEXT_FITXER_INVALID_DATA;
    }

    free(dades);

    if (error != NULL) {
        FITXER_enviarNack(fd, error);
    }

    return resultat;
}

void FITXER_enviarNack (int fd, char *motiu) {
    UTILS_printF(1, MSG_FITXER_RESPOSTA_INVALIDA);

    if (TRAMA_enviarTrama(fd, TIPUS_NACK, FLAG_ERROR, motiu) != TRAMA_OK) {
        UTILS_printF(1, MSG_FITXER_ERROR_ENVIAR);
    }
}

int FITXER_enviar (int fd, char *path, char *nom_fitxer, char *etiqueta, int es_resposta) {
    int mida;
    char md5[MIDA_MD5 + 1];
    int fd_fitxer;
    int resultat;

    mida = FITXER_midaFitxer(path);
    if (mida < 0) {
        return FITXER_ERROR;
    }

    if (FITXER_calcularMD5(path, md5) != FITXER_OK) {
        return FITXER_ERROR;
    }

    fd_fitxer = open(path, O_RDONLY);
    if (fd_fitxer < 0) {
        UTILS_printF(1, MSG_FITXER_ERROR_OBRIR);
        return FITXER_ERROR;
    }

    resultat = FITXER_enviarCapcalera(fd, nom_fitxer, etiqueta, mida, md5, es_resposta);

    if (resultat == FITXER_OK) {
        resultat = FITXER_esperarReady(fd);
    }

    if (resultat == FITXER_OK) {
        resultat = FITXER_enviarBlocs(fd, fd_fitxer, mida, es_resposta);
    }

    close(fd_fitxer);

    if (resultat == FITXER_OK) {
        resultat = FITXER_esperarAck(fd);
    }

    return resultat;
}

char **FITXER_separarCapcalera (char *dades, char *etiqueta_esperada, int *mida) {
    char **camps = NULL;
    char lletra;
    int n_camps = 0;
    int i;
    int separadors = 0;
    int valid = 1;

    *mida = 0;
    for (i = 0; dades[i] != '\0'; i++) {
        if (dades[i] == '&') {
            separadors++;
        }
    }
    if (separadors != N_CAMPS_FITXER - 1) {
        return NULL;
    }

    camps = UTILS_splitString(dades, '&', &n_camps);
    if (camps == NULL || n_camps != N_CAMPS_FITXER) {
        UTILS_freeTokens(camps, n_camps);
        return NULL;
    }

    if (strcmp(camps[0], etiqueta_esperada) != 0 || FITXER_nomFitxerValid(camps[1]) == FITXER_NOM_NO_VALID || UTILS_esNumeric(camps[2]) != TEXT_NUMERIC || strlen(camps[3]) != MIDA_MD5) {
        valid = 0;
    }
    

    for (i = 0; camps[3][i] != '\0'; i++) {
        lletra = camps[3][i];
        if (lletra >= 'A' && lletra <= 'F') {
            lletra = lletra - 'A' + 'a';
            camps[3][i] = lletra;
        }
        if ((lletra < '0' || lletra > '9') &&
            (lletra < 'a' || lletra > 'f')) {
            valid = 0;
        }
    }
    if (valid == 0) {
        UTILS_freeTokens(camps, n_camps);
        return NULL;
    }
    *mida = atoi(camps[2]);
    return camps;
}

int FITXER_nomFitxerValid (char *nom) {
    int i = 0;
    int valid = FITXER_NOM_VALID;

    if (nom[0] == '\0' || strcmp(nom, ".") == 0 || strcmp(nom, "..") == 0) {
        return FITXER_NOM_NO_VALID;
    }

    while (nom[i] != '\0' && valid == FITXER_NOM_VALID) {
        if (nom[i] == '/' || nom[i] == '\\') {
            valid = FITXER_NOM_NO_VALID;
        }
        i++;
    }

    return valid;
}

char **FITXER_rebreCapcalera (int fd, char *etiqueta_esperada, int *mida, int *flags_dades) {
    int tipus;
    int flags;
    int mida_dades;
    int mida_text;
    int flags_esperats = FLAG_PETICIO;
    int tipus_resposta = TIPUS_FITXER_READY;
    char *dades = NULL;
    char **camps = NULL;
    char *error = NULL;

    *mida = 0;
    *flags_dades = FLAG_BINARI;
    if (TRAMA_rebreTramaBinaria(fd, &tipus, &flags, &dades, &mida_dades) != TRAMA_OK) {
        UTILS_printF(1, MSG_FITXER_ERROR_REBRE);
        return NULL;
    }
    if (tipus == TIPUS_NACK) {
        UTILS_printF(1, MSG_FITXER_RESPOSTA_INVALIDA);
        free(dades);
        return NULL;
    }

    if (strcmp(etiqueta_esperada, TEXT_FITXER_OBJECT) == 0 || strcmp(etiqueta_esperada, TEXT_FITXER_SIGNED) == 0) {
        flags_esperats = FLAG_OK;
    }
    mida_text = strlen(dades);
    if (tipus != TIPUS_FITXER_CAPCALERA) {
        error = TEXT_FITXER_INVALID_TYPE;
        tipus_resposta = TIPUS_NACK;
    } else if (flags != flags_esperats) {
        error = TEXT_FITXER_INVALID_FLAGS;
    } else if (mida_dades != mida_text) {
        error = TEXT_FITXER_INVALID_DATA;
    } else {
        camps = FITXER_separarCapcalera(dades, etiqueta_esperada, mida);
        if (camps == NULL) {
            error = TEXT_FITXER_INVALID_DATA;
        }
    }
    free(dades);

    if (error != NULL) {
        UTILS_printF(1, MSG_FITXER_RESPOSTA_INVALIDA);
        if (TRAMA_enviarTrama(fd, tipus_resposta, FLAG_ERROR, error) != TRAMA_OK) {
            UTILS_printF(1, MSG_FITXER_ERROR_ENVIAR);
        }
        return NULL;
    }
    if (flags_esperats == FLAG_OK) {
        *flags_dades = FLAG_BINARI_RESPOSTA;
    }
    return camps;
}

int FITXER_prepararRecepcio (int fd, char *carpeta, char *prefix, char *nom_fitxer, char **path_rebut) {
    char *ruta = NULL;
    int fd_fitxer = FITXER_ERROR;
    int flags = FLAG_OK;
    char *resposta = TEXT_FITXER_READY;
    int resultat_trama;

    *path_rebut = NULL;

    if (asprintf(&ruta, FORMAT_FITXER_RUTA, carpeta, prefix, nom_fitxer) < 0) {
        UTILS_printF(1, MSG_SENSE_MEMORIA);
        ruta = NULL;
    } else {
        fd_fitxer = open(ruta, O_WRONLY | O_CREAT | O_TRUNC, PERMISOS_FITXER);
        if (fd_fitxer < 0) {
            UTILS_printF(1, MSG_FITXER_ERROR_OBRIR);
        }
    }

    if (fd_fitxer < 0) {
        flags = FLAG_ERROR;
        resposta = TEXT_FITXER_ERROR_CREAR;
    }

    resultat_trama = TRAMA_enviarTrama(fd, TIPUS_FITXER_READY, flags, resposta);
    if (resultat_trama != TRAMA_OK) {
        UTILS_printF(1, MSG_FITXER_ERROR_ENVIAR);
    }

    if (fd_fitxer < 0 || resultat_trama != TRAMA_OK) {
        if (fd_fitxer >= 0) {
            close(fd_fitxer);
            unlink(ruta);
        }
        free(ruta);
        return FITXER_ERROR;
    }

    *path_rebut = ruta;
    return fd_fitxer;
}

int FITXER_rebreBlocs (int fd, int fd_fitxer, int mida, int flags_dades) {
    int tipus, flags, mida_dades;
    int n_blocs, i;
    int escrits, resultat_write;
    char *dades = NULL;
    char *error = NULL;

    n_blocs = mida / MIDA_DADES;
    if (mida % MIDA_DADES != 0) {
        n_blocs++;
    }

    for (i = 0; i < n_blocs; i++) {
        if (TRAMA_rebreTramaBinaria(fd, &tipus, &flags, &dades, &mida_dades) != TRAMA_OK) {
            UTILS_printF(1, MSG_FITXER_ERROR_REBRE);
            return FITXER_ERROR;
        }

        error = NULL;
        if (tipus != TIPUS_FITXER_DADES) {
            error = TEXT_FITXER_INVALID_TYPE;
        } else if (flags != flags_dades) {
            error = TEXT_FITXER_INVALID_FLAGS;
        } else if (mida_dades == 0 || (i < n_blocs - 1 && mida_dades != MIDA_DADES)) {
            error = TEXT_TRAMA_INVALID_LENGTH;
        }
        if (error != NULL) {
            UTILS_printF(1, MSG_FITXER_RESPOSTA_INVALIDA);
            if (TRAMA_enviarTrama(fd, TIPUS_NACK, FLAG_ERROR, error) != TRAMA_OK) {
                UTILS_printF(1, MSG_FITXER_ERROR_ENVIAR);
            }
            free(dades);
            return FITXER_ERROR;
        }

        escrits = 0;
        while (escrits < mida_dades) {
            resultat_write = write(fd_fitxer, &dades[escrits], mida_dades - escrits);
            if (resultat_write <= 0) {
                UTILS_printF(1, MSG_FITXER_ERROR_ESCRIURE);
                free(dades);
                return FITXER_ERROR;
            }
            escrits = escrits + resultat_write;
        }
        free(dades);
    }
    return FITXER_OK;
}

int FITXER_verificarRecepcio (int fd, char *path, int mida_esperada, char *md5_esperat) {
    int mida_real;
    char md5_real[MIDA_MD5 + 1];
    int resultat = FITXER_OK;
    int flags = FLAG_OK;
    char *resposta = TEXT_FITXER_OK;

    mida_real = FITXER_midaFitxer(path);

    if (mida_real < 0) {
        resposta = TEXT_FITXER_ERROR_MIDA;
        resultat = FITXER_ERROR;
    } else if (mida_real != mida_esperada) {
        UTILS_printF(1, MSG_FITXER_MIDA_DIFERENT);
        resposta = TEXT_FITXER_ERROR_MIDA;
        resultat = FITXER_ERROR_MIDA;
    } else if (FITXER_calcularMD5(path, md5_real) != FITXER_OK) {
        resposta = TEXT_FITXER_ERROR_MD5;
        resultat = FITXER_ERROR;
    } else if (strcmp(md5_real, md5_esperat) != 0) {
        UTILS_printF(1, MSG_FITXER_MD5_DIFERENT);
        resposta = TEXT_FITXER_ERROR_MD5;
        resultat = FITXER_ERROR_MD5;
    }

    if (resultat != FITXER_OK) {
        flags = FLAG_ERROR;
    }

    if (TRAMA_enviarTrama(fd, TIPUS_FITXER_ACK, flags, resposta) != TRAMA_OK) {
        UTILS_printF(1, MSG_FITXER_ERROR_ENVIAR);
        return FITXER_ERROR;
    }

    return resultat;
}

int FITXER_rebre (int fd, char *carpeta, char *etiqueta_esperada, char *prefix, char **path_rebut) {
    char **camps = NULL;
    char *ruta = NULL;
    int mida;
    int flags_dades;
    int fd_fitxer;
    int resultat = FITXER_ERROR;

    *path_rebut = NULL;

    camps = FITXER_rebreCapcalera(fd, etiqueta_esperada, &mida, &flags_dades);
    if (camps == NULL) {
        return FITXER_ERROR;
    }

    fd_fitxer = FITXER_prepararRecepcio(fd, carpeta, prefix, camps[1], &ruta);

    if (fd_fitxer >= 0) {
        resultat = FITXER_rebreBlocs(fd, fd_fitxer, mida, flags_dades);
        close(fd_fitxer);

        if (resultat == FITXER_OK) {
            resultat = FITXER_verificarRecepcio(fd, ruta, mida, camps[3]);
        }
    }

    UTILS_freeTokens(camps, N_CAMPS_FITXER);

    if (resultat == FITXER_OK) {
        *path_rebut = ruta;
    } else {
        if (ruta != NULL) {
            unlink(ruta);
        }
        free(ruta);
    }

    return resultat;
}