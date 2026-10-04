/***********************************************
*
* @Proposit: Funcions auxiliars comunes als tres processos.
* @Autor/s: Arnau Ricart i Gerard Mas
* @Data creacio: 24/09/2026
* @Data ultima modificacio: 04/10/2026
*
************************************************/

//Define Guards
#ifndef _UTILS_H
#define _UTILS_H

//Llibreries del sistema
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

//Constants
#define TEXT_NO_NUMERIC 0
#define TEXT_NUMERIC    1

//Procediments i funcions

/***********************************************
*
* @Finalitat: Escriu una cadena pel descriptor indicat.
* @Parametres:  in: fd = descriptor de sortida.
*               in: text = cadena a escriure.
* @Retorn: ----.
*
************************************************/
void printF (int fd, char *text);

/***********************************************
*
* @Finalitat: Llegeix del descriptor fins al delimitador.
* @Parametres:  in: fd = descriptor a llegir.
*               in: delimitador = caracter on s'atura la lectura.
* @Retorn: Cadena reservada amb malloc, o NULL si s'ha acabat el fitxer.
*
************************************************/
char *readUntil (int fd, char delimitador);

/***********************************************
*
* @Finalitat: Separa una cadena pel delimitador indicat.
* @Parametres:  in: text = cadena a separar.
*               in: delimitador = caracter de tall.
*               out: n_trossos = nombre de trossos obtinguts.
* @Retorn: Array de cadenes reservat amb malloc, o NULL si no n'hi ha cap.
*
************************************************/
char **splitString (char *text, char delimitador, int *n_trossos);

/***********************************************
*
* @Finalitat: Allibera l'array retornat per splitString().
* @Parametres:  in/out: trossos = array a alliberar.
*               in: n_trossos = nombre de trossos.
* @Retorn: ----.
*
************************************************/
void freeTokens (char **trossos, int n_trossos);

/***********************************************
*
* @Finalitat: Comprova si una cadena nomes conte digits.
* @Parametres:  in: text = cadena a comprovar.
* @Retorn: TEXT_NUMERIC o TEXT_NO_NUMERIC.
*
************************************************/
int esNumeric (char *text);

#endif
