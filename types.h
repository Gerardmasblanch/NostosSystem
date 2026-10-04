/***********************************************
*
* @Proposit: Tipus propis compartits pels tres processos.
* @Autor/s: Arnau Ricart i Gerard Mas
* @Data creacio: 24/09/2026
* @Data ultima modificacio: 24/09/2026
*
************************************************/

//Define Guards
#ifndef _TYPES_H
#define _TYPES_H

//Tipus propis

//Aliment transportat per un Odysseus.
typedef struct {
    char *name;
    int   amount;
} Food;

//Configuracio llegida del fitxer odysseus.dat.
typedef struct {
    char *folder;
    char *name;
    char *ithaca_ip;
    int   ithaca_port;
    char *aeaea_ip;
    int   aeaea_port;
    int   gold;
    int   n_foods;
    Food *foods;
} OdysseusConfig;

//Configuracio llegida del fitxer ithaca.dat.
typedef struct {
    char *name;
    char *folder;
    char *ip;
    int   port;
} IthacaConfig;

//Encarrec llegit del fitxer voyages.dat. L'identificador el genera Ithaca.
typedef struct {
    int   id;
    char *object;
    char *file;
    char *destination;
    int   reward;
} Voyage;

//Connexio directa entre dues illes.
typedef struct {
    char *name;
    char *ip;
    int   port;
} Route;

//Configuracio llegida del fitxer island.dat.
typedef struct {
    char  *name;
    char  *folder;
    char  *ip;
    int    port;
    int    capacity;
    Route *routes;
    int    n_routes;
} IslandConfig;

//Registre del fitxer binari stock.db. Els tipus i l'ordre dels camps es
typedef struct {
    char name[100];
    int  amount;
    int  price;
} Product;

#endif
