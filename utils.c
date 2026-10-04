/***********************************************
*
* @Proposit: Implementacio de les funcions auxiliars compartides pels tres
*            processos del sistema.
* @Autor/s: Arnau Ricart i Gerard Mas
* @Data creacio: 24/09/2026
* @Data ultima modificacio: 24/09/2026
*
************************************************/

//Include del fitxer .h del mateix modul
#include "utils.h"

//Procediments i funcions

void printF (int fd, char *text) {
    write(fd, text, strlen(text));
}

char *readUntil (int fd, char delimitador) {
    char *buffer = NULL;
    char *nou = NULL;
    char lletra;
    int llargada = 0;
    int bytes_llegits = 0;

    while ((bytes_llegits = read(fd, &lletra, 1)) > 0 && lletra != delimitador) {
        nou = realloc(buffer, sizeof(char) * (llargada + 2));

        if (nou == NULL) {
            printF(2, MSG_SENSE_MEMORIA);
            free(buffer);
            return NULL;
        }

        buffer = nou;
        buffer[llargada] = lletra;
        llargada++;
    }

    //Final de fitxer sense haver llegit cap caracter.
    if (llargada == 0 && bytes_llegits <= 0) {
        free(buffer);
        return NULL;
    }

    //el delimitador ha arribat sense cap caracter previ.
    if (buffer == NULL) {
        buffer = malloc(sizeof(char));

        if (buffer == NULL) {
            printF(2, MSG_SENSE_MEMORIA);
            return NULL;
        }
    }

    buffer[llargada] = '\0';

    return buffer;
}

char **splitString (char *text, char delimitador, int *n_trossos) {
    char **trossos = NULL;
    char **nous = NULL;
    char *tros_actual = NULL;
    char *nou = NULL;
    int n_actuals = 0;
    int llargada = 0;
    int i = 0;
    int final = 0;

    *n_trossos = 0;

    if (text == NULL) {
        return NULL;
    }

    while (final == 0) {
        //Acumulem caracters mentre no arribi el delimitador ni el final.
        if (text[i] != delimitador && text[i] != '\0') {
            nou = realloc(tros_actual, sizeof(char) * (llargada + 2));

            if (nou == NULL) {
                printF(2, MSG_SENSE_MEMORIA);
                free(tros_actual);
                freeTokens(trossos, n_actuals);
                return NULL;
            }

            tros_actual = nou;
            tros_actual[llargada] = text[i];
            llargada++;
        } else {
            //Nomes guardem el tros si conte alguna cosa, aixi ens estalviem els trossos buits dels delimitadors repetits.
            if (llargada > 0) {
                tros_actual[llargada] = '\0';
                nous = realloc(trossos, sizeof(char *) * (n_actuals + 1));

                if (nous == NULL) {
                    printF(2, MSG_SENSE_MEMORIA);
                    free(tros_actual);
                    freeTokens(trossos, n_actuals);
                    return NULL;
                }

                trossos = nous;
                trossos[n_actuals] = tros_actual;
                n_actuals++;
                tros_actual = NULL;
                llargada = 0;
            }

            if (text[i] == '\0') {
                final = 1;
            }
        }

        i++;
    }

    *n_trossos = n_actuals;

    return trossos;
}

void freeTokens (char **trossos, int n_trossos) {
    int i;

    if (trossos == NULL) {
        return;
    }

    for (i = 0; i < n_trossos; i++) {
        free(trossos[i]);
    }

    free(trossos);
}

int esNumeric (char *text) {
    int i;

    if (text == NULL || text[0] == '\0') {
        return TEXT_NO_NUMERIC;
    }

    for (i = 0; text[i] != '\0'; i++) {
        if (text[i] < '0' || text[i] > '9') {
            return TEXT_NO_NUMERIC;
        }
    }

    return TEXT_NUMERIC;
}
