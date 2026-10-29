/***********************************************
*
* @Proposit: Enviament i recepcio de trames del protocol i obertura de sockets.
* @Autor/s: Arnau Ricart i Gerard Mas
* @Data creacio: 29/10/2026
* @Data ultima modificacio: 29/10/2026
*
************************************************/

//Define Guards
#ifndef _TRAMA_H
#define _TRAMA_H

//Llibreries del sistema
#include <arpa/inet.h>
#include <sys/socket.h>

//Llibreries propies
#include "utils.h"

//Constants
#define ERR_CODI -1
#define MAX_CONNEXIONS 10

#define TRAMA_ERROR 0
#define TRAMA_OK    1

#define MIDA_TRAMA 256
#define MIDA_DADES 253

#define POS_TIPUS 0
#define POS_FLAGS 1
#define POS_MIDA  2
#define POS_DADES 3

#define TIPUS_CONNECT       0x01
#define TIPUS_LIST_VOYAGES  0x02
#define TIPUS_ACCEPT        0x03
#define TIPUS_CLAIM         0x04
#define TIPUS_SORTIR_ITHACA 0x05
#define TIPUS_ARRIBADA      0x10
#define TIPUS_LIST_MARKET   0x11
#define TIPUS_BUY           0x12
#define TIPUS_SELL          0x13
#define TIPUS_BUY_MAP       0x14
#define TIPUS_DELIVER       0x15
#define TIPUS_SORTIR_ILLA   0x16
#define TIPUS_NACK          0x7F

#define FLAG_PETICIO         0x00
#define FLAG_OK              0x80
#define FLAG_ERROR           0xC0
#define FLAG_BINARI          0x20
#define FLAG_BINARI_RESPOSTA 0xA0

//Procediments i funcions

/***********************************************
*
* @Finalitat: Munta una trama de 256 bytes i l'envia amb un sol write.
* @Parametres:  in: fd = socket on s'envia.
*               in: tipus = camp TYPE de la trama.
*               in: flags = camp FLAGS de la trama.
*               in: dades = text del camp DATA.
* @Retorn: TRAMA_OK si s'ha enviat sencera, TRAMA_ERROR si no.
*
************************************************/
int enviarTrama (int fd, int tipus, int flags, char *dades);

/***********************************************
*
* @Finalitat: Llegeix una trama de 256 bytes amb un sol read i en separa els camps.
* @Parametres:  in: fd = socket d'on es llegeix.
*               out: tipus = camp TYPE rebut.
*               out: flags = camp FLAGS rebut.
*               out: dades = text del camp DATA, reservat amb malloc.
* @Retorn: TRAMA_OK si s'ha rebut sencera, TRAMA_ERROR si no.
*
************************************************/
int rebreTrama (int fd, int *tipus, int *flags, char **dades);

/***********************************************
*
* @Finalitat: Crea el socket del servidor i el posa a escoltar.
* @Parametres:  in: ip = IP on escolta el servidor.
*               in: port = port on escolta el servidor.
* @Retorn: Descriptor del socket, o ERR_CODI si hi ha hagut un error.
*
************************************************/
int obrirServidor (char *ip, int port);

/***********************************************
*
* @Finalitat: Es connecta a un servidor.
* @Parametres:  in: ip = IP del servidor.
*               in: port = port del servidor.
* @Retorn: Descriptor del socket, o ERR_CODI si hi ha hagut un error.
*
************************************************/
int connectarServidor (char *ip, int port);

#endif