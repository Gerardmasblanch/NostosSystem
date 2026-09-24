/***********************************************
*
* @Proposit: Funcions auxiliars compartides pels tres processos: escriptura
*            per descriptor, lectura de fitxers i tractament de cadenes.
* @Autor/s: Arnau Ricart i Gerard Mas
* @Data creacio: 24/09/2026
* @Data ultima modificacio: 24/09/2026
*
************************************************/

//Define Guards
#ifndef _UTILS_H
#define _UTILS_H

//Macros de compilacio: han d'estar definides abans de cap llibreria.
#define _GNU_SOURCE
#define _XOPEN_SOURCE 500
#define _POSIX_C_SOURCE 1

//Llibreries del sistema
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

//Constants
#define VALOR_NO_NUMERIC -1

//Procediments i funcions

/***********************************************
*
* @Finalitat: Escriu una cadena de caracters al descriptor indicat.
* @Parametres:  in: fd = descriptor on s'ha d'escriure.
*               in: text = cadena que es vol escriure.
* @Retorn: ----.
*
************************************************/
void printF (int fd, const char *text);

/***********************************************
*
* @Finalitat: Llegeix del descriptor fins trobar el delimitador o el final
*             del fitxer. El delimitador no forma part del resultat.
* @Parametres:  in: fd = descriptor del fitxer que es vol llegir.
*               in: delimitador = caracter que marca el final de la lectura.
* @Retorn: Cadena reservada dinamicament amb el text llegit, o NULL si s'ha
*          arribat al final del fitxer sense llegir res. Una linia buida
*          retorna una cadena buida, no NULL.
*
************************************************/
char *readUntil (int fd, char delimitador);

/***********************************************
*
* @Finalitat: Separa una cadena en trossos segons el delimitador indicat.
*             Els delimitadors repetits no generen trossos buits.
* @Parametres:  in: text = cadena que es vol separar.
*               in: delimitador = caracter pel qual es talla la cadena.
*               out: n_trossos = nombre de trossos obtinguts.
* @Retorn: Array de cadenes reservat dinamicament, o NULL si no hi ha cap
*          tros. Cal alliberar-lo amb freeTokens().
*
************************************************/
char **splitString (const char *text, char delimitador, int *n_trossos);

/***********************************************
*
* @Finalitat: Allibera l'array de cadenes retornat per splitString().
* @Parametres:  in/out: trossos = array que es vol alliberar.
*               in: n_trossos = nombre de trossos que conte l'array.
* @Retorn: ----.
*
************************************************/
void freeTokens (char **trossos, int n_trossos);

/***********************************************
*
* @Finalitat: Converteix una cadena en un enter, comprovant que nomes
*             contingui digits.
* @Parametres:  in: text = cadena que es vol convertir.
* @Retorn: El valor enter corresponent, o VALOR_NO_NUMERIC si la cadena
*          es buida o conte algun caracter que no es un digit.
*
************************************************/
int parseInt (const char *text);

#endif
