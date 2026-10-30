/***********************************************
*
* @Proposit: Implementacio de l'estat del navegant Odysseus.
* @Autor/s: Arnau Ricart i Gerard Mas
* @Data creacio: 29/10/2026
* @Data ultima modificacio: 29/10/2026
*
************************************************/

//Include del fitxer .h del mateix modul
#include "navegant.h"

//Procediments i funcions

int inicialitzarNavegant (Navegant *navegant) {
    navegant->estat = ESTAT_ITHACA;
    navegant->fd_connexio = ERR_CODI;
    navegant->te_viatge = SENSE_VIATGE;
    navegant->viatge.object = NULL;
    navegant->viatge.file = NULL;
    navegant->viatge.destination = NULL;
    navegant->mapa = NULL;
    navegant->n_illes = 0;
    navegant->adreces = NULL;
    navegant->n_adreces = 0;

    navegant->ubicacio = copiarText(TEXT_ITHACA);
    if (navegant->ubicacio == NULL) {
        printF(1, MSG_SENSE_MEMORIA);
        return ERR_CODI;
    }

    if (afegirIllaMapa(navegant, TEXT_AEAEA) == ERR_CODI) {
        return ERR_CODI;
    }

    if (afegirAdreca(navegant, TEXT_AEAEA, navegant->config.aeaea_ip, navegant->config.aeaea_port) == ERR_CODI) {
        return ERR_CODI;
    }
    
    return 0;
}

void alliberarNavegant (Navegant *navegant) {
    int i;
    int j;

    if (navegant->fd_connexio != ERR_CODI) {
        close(navegant->fd_connexio);
    }

    free(navegant->ubicacio);
    free(navegant->viatge.object);
    free(navegant->viatge.destination);

    for (i = 0; i < navegant->n_illes; i++) {
        for (j = 0; j < navegant->mapa[i].known_island_count; j++) {
            free(navegant->mapa[i].known_islands[j]);
        }
        free(navegant->mapa[i].known_islands);
        free(navegant->mapa[i].name);
    }
    free(navegant->mapa);

    for (i = 0; i < navegant->n_adreces; i++) {
        free(navegant->adreces[i].name);
        free(navegant->adreces[i].ip);
    }
    free(navegant->adreces);

    freeOdysseusConfig(&navegant->config);
}

int totalProvisions (Navegant *navegant) {
    int i;
    int total = 0;

    for (i = 0; i < navegant->config.n_foods; i++) {
        total = total + navegant->config.foods[i].amount;
    }

    return total;
}

void mostrarCarrega (Navegant *navegant) {
    char *missatge = NULL;
    int i;

    printF(1, MSG_CARREGA);

    for (i = 0; i < navegant->config.n_foods; i++) {
        if (navegant->config.foods[i].amount > 0) {
            if (asprintf(&missatge, FORMAT_ALIMENT, navegant->config.foods[i].name, navegant->config.foods[i].amount) < 0) {
                printF(1, MSG_SENSE_MEMORIA);
                return;
            }

            printF(1, missatge);
            free(missatge);
        }
    }

    if (asprintf(&missatge, FORMAT_TOTAL_PROVISIONS, totalProvisions(navegant)) < 0) {
        printF(1, MSG_SENSE_MEMORIA);
        return;
    }

    printF(1, missatge);
    free(missatge);
}

void mostrarStatus (Navegant *navegant) {
    char *nom = NULL;
    char *missatge = NULL;
    int i;

    nom = copiarText(navegant->config.name);
    if (nom == NULL) {
        printF(1, MSG_SENSE_MEMORIA);
        return;
    }

    for (i = 0; nom[i] != '\0'; i++) {
        if (nom[i] >= 'a' && nom[i] <= 'z') {
            nom[i] = nom[i] - 'a' + 'A';
        }
    }

    if (asprintf(&missatge, FORMAT_CAPCALERA_STATUS, nom, navegant->config.gold) < 0) {
        free(nom);
        printF(1, MSG_SENSE_MEMORIA);
        return;
    }
    free(nom);
    printF(1, missatge);
    free(missatge);

    mostrarCarrega(navegant);

    if (navegant->te_viatge == AMB_VIATGE) {
        if (asprintf(&missatge, FORMAT_VIATGE_STATUS, navegant->viatge.object, navegant->viatge.destination, navegant->viatge.reward) < 0) {
            printF(1, MSG_SENSE_MEMORIA);
            return;
        }
        printF(1, missatge);
        free(missatge);
    } else {
        printF(1, MSG_STATUS_SENSE_VIATGE);
    }

    if (asprintf(&missatge, FORMAT_UBICACIO_STATUS, navegant->ubicacio) < 0) {
        printF(1, MSG_SENSE_MEMORIA);
        return;
    }
    printF(1, missatge);
    free(missatge);
}

int buscarAliment (Navegant *navegant, char *nom) {
    int i;
    int index = ERR_CODI;

    for (i = 0; i < navegant->config.n_foods; i++) {
        if (strcasecmp(navegant->config.foods[i].name, nom) == 0) {
            index = i;
        }
    }

    return index;
}

int prepararAliment (Navegant *navegant, char *nom) {
    Food *nous_aliments = NULL;
    char *nom_copia = NULL;
    int index;

    index = buscarAliment(navegant, nom);
    if (index != ERR_CODI) {
        return index;
    }

    nom_copia = copiarText(nom);
    if (nom_copia == NULL) {
        printF(1, MSG_SENSE_MEMORIA);
        return ERR_CODI;
    }

    nous_aliments = realloc(navegant->config.foods, sizeof(Food) * (navegant->config.n_foods + 1));

    if (nous_aliments == NULL) {
        free(nom_copia);
        printF(1, MSG_SENSE_MEMORIA);
        return ERR_CODI;
    }

    navegant->config.foods = nous_aliments;
    index = navegant->config.n_foods;
    navegant->config.foods[index].name = nom_copia;
    navegant->config.foods[index].amount = 0;
    navegant->config.n_foods++;

    return index;
}

int guardarCompra (Navegant *navegant, int index, int quantitat, char *dades, int *cost) {
    char **trossos = NULL;
    int n_trossos;
    int quantitat_rebuda;
    int cost_rebut;
    int resultat = TRAMA_ERROR;

    trossos = splitString(dades, '&', &n_trossos);

    if (n_trossos == 4) {
        if (strcmp(trossos[0], TEXT_COMPRA_OK) == 0 &&
            strcasecmp(trossos[1], navegant->config.foods[index].name) == 0 &&
            esNumeric(trossos[2]) == TEXT_NUMERIC &&
            esNumeric(trossos[3]) == TEXT_NUMERIC) {

            quantitat_rebuda = atoi(trossos[2]);
            cost_rebut = atoi(trossos[3]);

            if (quantitat_rebuda == quantitat && quantitat > 0 &&
                cost_rebut >= 0 && cost_rebut <= navegant->config.gold) {

                navegant->config.foods[index].amount = navegant->config.foods[index].amount + quantitat_rebuda;
                navegant->config.gold = navegant->config.gold - cost_rebut;
                *cost = cost_rebut;
                resultat = TRAMA_OK;
            }
        }
    }

    freeTokens(trossos, n_trossos);

    return resultat;
}

int guardarVenda (Navegant *navegant, int index, int quantitat, char *dades, int *ingressos) {
    char **trossos = NULL;
    int n_trossos;
    int quantitat_rebuda;
    int ingressos_rebuts;
    int resultat = TRAMA_ERROR;

    trossos = splitString(dades, '&', &n_trossos);

    if (n_trossos == 4) {
        if (strcmp(trossos[0], TEXT_VENDA_OK) == 0 &&
            strcasecmp(trossos[1], navegant->config.foods[index].name) == 0 &&
            esNumeric(trossos[2]) == TEXT_NUMERIC &&
            esNumeric(trossos[3]) == TEXT_NUMERIC) {

            quantitat_rebuda = atoi(trossos[2]);
            ingressos_rebuts = atoi(trossos[3]);

            if (quantitat_rebuda == quantitat && quantitat > 0 && quantitat_rebuda <= navegant->config.foods[index].amount && ingressos_rebuts >= 0) {

                navegant->config.foods[index].amount = navegant->config.foods[index].amount - quantitat_rebuda;
                navegant->config.gold = navegant->config.gold + ingressos_rebuts;
                *ingressos = ingressos_rebuts;
                resultat = TRAMA_OK;
            }
        }
    }

    freeTokens(trossos, n_trossos);

    return resultat;
}

void consumirRacio (Navegant *navegant, int quilos) {
    int i = 0;
    int pendent = quilos;

    while (pendent > 0 && i < navegant->config.n_foods) {
        if (navegant->config.foods[i].amount >= pendent) {
            navegant->config.foods[i].amount = navegant->config.foods[i].amount - pendent;
            pendent = 0;
        } else {
            pendent = pendent - navegant->config.foods[i].amount;
            navegant->config.foods[i].amount = 0;
        }
        i++;
    }
}

int buscarAdreca (Navegant *navegant, char *nom) {
    int i;
    int index = ERR_CODI;

    for (i = 0; i < navegant->n_adreces; i++) {
        if (strcasecmp(navegant->adreces[i].name, nom) == 0) {
            index = i;
        }
    }

    return index;
}

int esRutaDirecta (Navegant *navegant, char *desti) {
    int i;
    int j;
    int directa = RUTA_NO_DIRECTA;

    for (i = 0; i < navegant->n_illes; i++) {
        if (strcasecmp(navegant->mapa[i].name, navegant->ubicacio) == 0) {
            for (j = 0; j < navegant->mapa[i].known_island_count; j++) {
                if (strcasecmp(navegant->mapa[i].known_islands[j], desti) == 0) {
                    directa = RUTA_DIRECTA;
                }
            }
        }
    }

    return directa;
}

int canviarUbicacio (Navegant *navegant, char *nom) {
    char *nova = NULL;
    int resultat = ERR_CODI;

    nova = copiarText(nom);

    if (nova == NULL) {
        printF(1, MSG_SENSE_MEMORIA);
    } else {
        free(navegant->ubicacio);
        navegant->ubicacio = nova;
        resultat = 0;
    }

    return resultat;
}

int buscarIllaMapa (Navegant *navegant, char *nom) {
    int i;
    int index = ERR_CODI;

    for (i = 0; i < navegant->n_illes; i++) {
        if (strcasecmp(navegant->mapa[i].name, nom) == 0) {
            index = i;
        }
    }

    return index;
}


int afegirIllaMapa (Navegant *navegant, char *nom) {
    SPHRAGIS_Island *nou_mapa = NULL;
    int index;

    index = buscarIllaMapa(navegant, nom);
    if (index != ERR_CODI) {
        return index;
    }

    nou_mapa = realloc(navegant->mapa, sizeof(SPHRAGIS_Island) * (navegant->n_illes + 1));
    if (nou_mapa == NULL) {
        printF(1, MSG_SENSE_MEMORIA);
        return ERR_CODI;
    }

    navegant->mapa = nou_mapa;
    index = navegant->n_illes;

    navegant->mapa[index].name = copiarText(nom);
    if (navegant->mapa[index].name == NULL) {
        printF(1, MSG_SENSE_MEMORIA);
        return ERR_CODI;
    }

    navegant->mapa[index].known_islands = NULL;
    navegant->mapa[index].known_island_count = 0;
    navegant->n_illes++;

    return index;
}

int afegirConnexio (Navegant *navegant, int index, char *desti) {
    char **noves_connexions = NULL;
    int i;
    int existeix = 0;
    int resultat = 0;

    for (i = 0; i < navegant->mapa[index].known_island_count; i++) {
        if (strcasecmp(navegant->mapa[index].known_islands[i], desti) == 0) {
            existeix = 1;
        }
    }

    if (existeix == 0) {
        noves_connexions = realloc(navegant->mapa[index].known_islands, sizeof(char *) * (navegant->mapa[index].known_island_count + 1));

        if (noves_connexions == NULL) {
            printF(1, MSG_SENSE_MEMORIA);
            resultat = ERR_CODI;
        } else {
            navegant->mapa[index].known_islands = noves_connexions;
            navegant->mapa[index].known_islands[navegant->mapa[index].known_island_count] = copiarText(desti);

            if (navegant->mapa[index].known_islands[navegant->mapa[index].known_island_count] == NULL) {
                printF(1, MSG_SENSE_MEMORIA);
                resultat = ERR_CODI;
            } else {
                navegant->mapa[index].known_island_count++;
            }
        }
    }

    return resultat;
}

int afegirRuta (Navegant *navegant, char *origen, char *desti) {
    int index_origen;
    int index_desti;
    int resultat = ERR_CODI;

    index_origen = afegirIllaMapa(navegant, origen);
    index_desti = afegirIllaMapa(navegant, desti);

    if (index_origen != ERR_CODI && index_desti != ERR_CODI) {
        if (afegirConnexio(navegant, index_origen, desti) == 0 && afegirConnexio(navegant, index_desti, origen) == 0) {
            resultat = 0;
        }
    }

    return resultat;
}

int afegirAdreca (Navegant *navegant, char *nom, char *ip, int port) {
    Route *noves_adreces = NULL;
    int index;

    if (buscarAdreca(navegant, nom) != ERR_CODI) {
        return 0;
    }

    noves_adreces = realloc(navegant->adreces, sizeof(Route) * (navegant->n_adreces + 1));
    if (noves_adreces == NULL) {
        printF(1, MSG_SENSE_MEMORIA);
        return ERR_CODI;
    }

    navegant->adreces = noves_adreces;
    index = navegant->n_adreces;

    navegant->adreces[index].name = copiarText(nom);
    navegant->adreces[index].ip = copiarText(ip);

    if (navegant->adreces[index].name == NULL || navegant->adreces[index].ip == NULL) {
        free(navegant->adreces[index].name);
        free(navegant->adreces[index].ip);
        printF(1, MSG_SENSE_MEMORIA);
        return ERR_CODI;
    }

    navegant->adreces[index].port = port;
    navegant->n_adreces++;

    return 0;
}
