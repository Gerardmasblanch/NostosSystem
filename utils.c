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
    char lletra;
    int llargada = 0;
    int bytes_llegits = 0;

    while ((bytes_llegits = read(fd, &lletra, 1)) > 0 && lletra != delimitador) {
        buffer = realloc(buffer, sizeof(char) * (llargada + 2));
        buffer[llargada] = lletra;
        llargada++;
    }

    //Final de fitxer sense haver llegit cap caracter.
    if (llargada == 0 && bytes_llegits <= 0) {
        return NULL;
    }

    //el delimitador ha arribat sense cap caracter previ.
    if (buffer == NULL) {
        buffer = malloc(sizeof(char));
    }

    buffer[llargada] = '\0';

    return buffer;
}

char **splitString (char *text, char delimitador, int *n_trossos) {
    char **trossos = NULL;
    char *tros_actual = NULL;
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
            tros_actual = realloc(tros_actual, sizeof(char) * (llargada + 2));
            tros_actual[llargada] = text[i];
            llargada++;
        } else {
            //Nomes guardem el tros si conte alguna cosa, aixi ens estalviem els trossos buits dels delimitadors repetits.
            if (llargada > 0) {
                tros_actual[llargada] = '\0';
                trossos = realloc(trossos, sizeof(char *) * (n_actuals + 1));
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
