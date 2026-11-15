/***********************************************
*
* @Proposit: Implementacio de l'estat del navegant Odysseus.
* @Autor/s: Arnau Ricart i Gerard Mas
* @Data creacio: 29/10/2026
* @Data ultima modificacio: 15/11/2026
*
************************************************/

//Include del fitxer .h del mateix modul
#include "navegant.h"

//Procediments i funcions

int NAVEGANT_inicialitzarNavegant (Navegant *navegant) {
    navegant->estat = ESTAT_ITHACA;
    navegant->fd_connexio = ERR_CODI;
    navegant->te_viatge = SENSE_VIATGE;
    navegant->viatge.object = NULL;
    navegant->viatge.file = NULL;
    navegant->viatge.destination = NULL;
    navegant->viatge_lliurat = VIATGE_NO_LLIURAT;
    navegant->fitxer_segellat = NULL;
    navegant->mapa = NULL;
    navegant->n_illes = 0;
    navegant->adreces = NULL;
    navegant->n_adreces = 0;
    navegant->mercat_consultat = 0;

    navegant->ubicacio = UTILS_copiarText(TEXT_ITHACA);
    if (navegant->ubicacio == NULL) {
        UTILS_printF(1, MSG_SENSE_MEMORIA);
        return ERR_CODI;
    }

    if (NAVEGANT_afegirIllaMapa(navegant, TEXT_AEAEA) == ERR_CODI) {
        return ERR_CODI;
    }

    if (NAVEGANT_afegirAdreca(navegant, TEXT_AEAEA, navegant->config.aeaea_ip, navegant->config.aeaea_port) == ERR_CODI) {
        return ERR_CODI;
    }
    
    return 0;
}

void NAVEGANT_alliberarNavegant (Navegant *navegant) {
    int i;
    int j;

    if (navegant->fd_connexio != ERR_CODI) {
        close(navegant->fd_connexio);
    }

    free(navegant->ubicacio);
    NAVEGANT_esborrarViatge(navegant);

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

    CONFIG_freeOdysseusConfig(&navegant->config);
}

void NAVEGANT_esborrarViatge (Navegant *navegant) {
    free(navegant->viatge.object);
    free(navegant->viatge.destination);
    free(navegant->viatge.file);
    free(navegant->fitxer_segellat);

    navegant->viatge.object = NULL;
    navegant->viatge.destination = NULL;
    navegant->viatge.file = NULL;
    navegant->fitxer_segellat = NULL;
    navegant->te_viatge = SENSE_VIATGE;
    navegant->viatge_lliurat = VIATGE_NO_LLIURAT;
}

int NAVEGANT_totalProvisions (Navegant *navegant) {
    int i;
    int total = 0;

    for (i = 0; i < navegant->config.n_foods; i++) {
        total = total + navegant->config.foods[i].amount;
    }

    return total;
}

void NAVEGANT_mostrarCarrega (Navegant *navegant) {
    char *missatge = NULL;
    int i;

    UTILS_printF(1, MSG_CARREGA);

    for (i = 0; i < navegant->config.n_foods; i++) {
        if (navegant->config.foods[i].amount > 0) {
            if (asprintf(&missatge, FORMAT_ALIMENT, navegant->config.foods[i].name, navegant->config.foods[i].amount) < 0) {
                UTILS_printF(1, MSG_SENSE_MEMORIA);
                return;
            }

            UTILS_printF(1, missatge);
            free(missatge);
        }
    }

    if (asprintf(&missatge, FORMAT_TOTAL_PROVISIONS, NAVEGANT_totalProvisions(navegant)) < 0) {
        UTILS_printF(1, MSG_SENSE_MEMORIA);
        return;
    }

    UTILS_printF(1, missatge);
    free(missatge);
}

void NAVEGANT_mostrarViatge (Navegant *navegant) {
    char *missatge = NULL;

    if (navegant->te_viatge != AMB_VIATGE) {
        UTILS_printF(1, MSG_STATUS_SENSE_VIATGE);
        return;
    }

    if (asprintf(&missatge, FORMAT_VIATGE_STATUS, navegant->viatge.object, navegant->viatge.destination, navegant->viatge.reward) < 0) {
        UTILS_printF(1, MSG_SENSE_MEMORIA);
        return;
    }
    UTILS_printF(1, missatge);
    free(missatge);

    if (navegant->viatge.file == NULL) {
        return;
    }

    if (asprintf(&missatge, FORMAT_OBJECTE_STATUS, navegant->viatge.file + strlen(navegant->config.folder) + 1) < 0) {
        UTILS_printF(1, MSG_SENSE_MEMORIA);
        return;
    }
    UTILS_printF(1, missatge);
    free(missatge);
}

void NAVEGANT_mostrarStatus (Navegant *navegant) {
    char *nom = NULL;
    char *missatge = NULL;
    int i;

    nom = UTILS_copiarText(navegant->config.name);
    if (nom == NULL) {
        UTILS_printF(1, MSG_SENSE_MEMORIA);
        return;
    }

    for (i = 0; nom[i] != '\0'; i++) {
        if (nom[i] >= 'a' && nom[i] <= 'z') {
            nom[i] = nom[i] - 'a' + 'A';
        }
    }

    if (asprintf(&missatge, FORMAT_CAPCALERA_STATUS, nom, navegant->config.gold) < 0) {
        free(nom);
        UTILS_printF(1, MSG_SENSE_MEMORIA);
        return;
    }
    free(nom);
    UTILS_printF(1, missatge);
    free(missatge);

    NAVEGANT_mostrarCarrega(navegant);
    NAVEGANT_mostrarViatge(navegant);

    if (navegant->estat == ESTAT_ITHACA) {
        UTILS_printF(1, MSG_UBICACIO_DESCONEGUDA);
        return;
    }

    if (navegant->estat == ESTAT_ESPERANT) {
        if (asprintf(&missatge, FORMAT_UBICACIO_ESPERANT, navegant->ubicacio) < 0) {
            UTILS_printF(1, MSG_SENSE_MEMORIA);
            return;
        }
        UTILS_printF(1, missatge);
        free(missatge);
        return;
    }

    if (asprintf(&missatge, FORMAT_UBICACIO_STATUS, navegant->ubicacio) < 0) {
        UTILS_printF(1, MSG_SENSE_MEMORIA);
        return;
    }
    UTILS_printF(1, missatge);
    free(missatge);
}

int NAVEGANT_buscarAliment (Navegant *navegant, char *nom) {
    int i;
    int index = ERR_CODI;

    for (i = 0; i < navegant->config.n_foods; i++) {
        if (strcasecmp(navegant->config.foods[i].name, nom) == 0) {
            index = i;
        }
    }

    return index;
}

int NAVEGANT_prepararAliment (Navegant *navegant, char *nom) {
    Food *nous_aliments = NULL;
    char *nom_copia = NULL;
    int index;

    index = NAVEGANT_buscarAliment(navegant, nom);
    if (index != ERR_CODI) {
        return index;
    }

    nom_copia = UTILS_copiarText(nom);
    if (nom_copia == NULL) {
        UTILS_printF(1, MSG_SENSE_MEMORIA);
        return ERR_CODI;
    }

    nous_aliments = realloc(navegant->config.foods, sizeof(Food) * (navegant->config.n_foods + 1));

    if (nous_aliments == NULL) {
        free(nom_copia);
        UTILS_printF(1, MSG_SENSE_MEMORIA);
        return ERR_CODI;
    }

    navegant->config.foods = nous_aliments;
    index = navegant->config.n_foods;
    navegant->config.foods[index].name = nom_copia;
    navegant->config.foods[index].amount = 0;
    navegant->config.n_foods++;

    return index;
}

int NAVEGANT_guardarCompra (Navegant *navegant, int index, int quantitat, char *dades, int *cost) {
    char **trossos = NULL;
    int n_trossos;
    int quantitat_rebuda;
    int cost_rebut;
    int resultat = TRAMA_ERROR;

    trossos = UTILS_splitString(dades, '&', &n_trossos);

    if (n_trossos == 4) {
        if (strcmp(trossos[0], TEXT_COMPRA_OK) == 0 &&
            strcasecmp(trossos[1], navegant->config.foods[index].name) == 0 &&
            UTILS_esNumeric(trossos[2]) == TEXT_NUMERIC &&
            UTILS_esNumeric(trossos[3]) == TEXT_NUMERIC) {

            quantitat_rebuda = atoi(trossos[2]);
            cost_rebut = atoi(trossos[3]);

            if (quantitat_rebuda == quantitat && quantitat > 0 &&
                cost_rebut >= 0 && cost_rebut <= navegant->config.gold) {

                strcpy(navegant->config.foods[index].name, trossos[1]);
                navegant->config.foods[index].amount = navegant->config.foods[index].amount + quantitat_rebuda;
                navegant->config.gold = navegant->config.gold - cost_rebut;
                *cost = cost_rebut;
                resultat = TRAMA_OK;
            }
        }
    }

    UTILS_freeTokens(trossos, n_trossos);

    return resultat;
}

int NAVEGANT_guardarVenda (Navegant *navegant, int index, int quantitat, char *dades, int *ingressos) {
    char **trossos = NULL;
    int n_trossos;
    int quantitat_rebuda;
    int ingressos_rebuts;
    int resultat = TRAMA_ERROR;

    trossos = UTILS_splitString(dades, '&', &n_trossos);

    if (n_trossos == 4) {
        if (strcmp(trossos[0], TEXT_VENDA_OK) == 0 &&
            strcasecmp(trossos[1], navegant->config.foods[index].name) == 0 &&
            UTILS_esNumeric(trossos[2]) == TEXT_NUMERIC &&
            UTILS_esNumeric(trossos[3]) == TEXT_NUMERIC) {

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

    UTILS_freeTokens(trossos, n_trossos);

    return resultat;
}

void NAVEGANT_consumirRacio (Navegant *navegant, int quilos) {
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

int NAVEGANT_buscarAdreca (Navegant *navegant, char *nom) {
    int i;
    int index = ERR_CODI;

    for (i = 0; i < navegant->n_adreces; i++) {
        if (strcasecmp(navegant->adreces[i].name, nom) == 0) {
            index = i;
        }
    }

    return index;
}

int NAVEGANT_esRutaDirecta (Navegant *navegant, char *desti) {
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

int NAVEGANT_canviarUbicacio (Navegant *navegant, char *nom) {
    char *nova = NULL;
    int resultat = ERR_CODI;

    nova = UTILS_copiarText(nom);

    if (nova == NULL) {
        UTILS_printF(1, MSG_SENSE_MEMORIA);
    } else {
        free(navegant->ubicacio);
        navegant->ubicacio = nova;
        resultat = 0;
    }

    return resultat;
}

int NAVEGANT_buscarIllaMapa (Navegant *navegant, char *nom) {
    int i;
    int index = ERR_CODI;

    for (i = 0; i < navegant->n_illes; i++) {
        if (strcasecmp(navegant->mapa[i].name, nom) == 0) {
            index = i;
        }
    }

    return index;
}


int NAVEGANT_afegirIllaMapa (Navegant *navegant, char *nom) {
    SPHRAGIS_Island *nou_mapa = NULL;
    int index;

    index = NAVEGANT_buscarIllaMapa(navegant, nom);
    if (index != ERR_CODI) {
        return index;
    }

    nou_mapa = realloc(navegant->mapa, sizeof(SPHRAGIS_Island) * (navegant->n_illes + 1));
    if (nou_mapa == NULL) {
        UTILS_printF(1, MSG_SENSE_MEMORIA);
        return ERR_CODI;
    }

    navegant->mapa = nou_mapa;
    index = navegant->n_illes;

    navegant->mapa[index].name = UTILS_copiarText(nom);
    if (navegant->mapa[index].name == NULL) {
        UTILS_printF(1, MSG_SENSE_MEMORIA);
        return ERR_CODI;
    }

    navegant->mapa[index].known_islands = NULL;
    navegant->mapa[index].known_island_count = 0;
    navegant->n_illes++;

    return index;
}

int NAVEGANT_afegirConnexio (Navegant *navegant, int index, char *desti) {
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
            UTILS_printF(1, MSG_SENSE_MEMORIA);
            resultat = ERR_CODI;
        } else {
            navegant->mapa[index].known_islands = noves_connexions;
            navegant->mapa[index].known_islands[navegant->mapa[index].known_island_count] = UTILS_copiarText(desti);

            if (navegant->mapa[index].known_islands[navegant->mapa[index].known_island_count] == NULL) {
                UTILS_printF(1, MSG_SENSE_MEMORIA);
                resultat = ERR_CODI;
            } else {
                navegant->mapa[index].known_island_count++;
            }
        }
    }

    return resultat;
}

int NAVEGANT_afegirRuta (Navegant *navegant, char *origen, char *desti) {
    int index_origen;
    int index_desti;
    int resultat = ERR_CODI;

    index_origen = NAVEGANT_afegirIllaMapa(navegant, origen);
    index_desti = NAVEGANT_afegirIllaMapa(navegant, desti);

    if (index_origen != ERR_CODI && index_desti != ERR_CODI) {
        if (NAVEGANT_afegirConnexio(navegant, index_origen, desti) == 0 && NAVEGANT_afegirConnexio(navegant, index_desti, origen) == 0) {
            resultat = 0;
        }
    }

    return resultat;
}

int NAVEGANT_afegirAdreca (Navegant *navegant, char *nom, char *ip, int port) {
    Route *noves_adreces = NULL;
    int index;

    if (NAVEGANT_buscarAdreca(navegant, nom) != ERR_CODI) {
        return 0;
    }

    noves_adreces = realloc(navegant->adreces, sizeof(Route) * (navegant->n_adreces + 1));
    if (noves_adreces == NULL) {
        UTILS_printF(1, MSG_SENSE_MEMORIA);
        return ERR_CODI;
    }

    navegant->adreces = noves_adreces;
    index = navegant->n_adreces;

    navegant->adreces[index].name = UTILS_copiarText(nom);
    navegant->adreces[index].ip = UTILS_copiarText(ip);

    if (navegant->adreces[index].name == NULL || navegant->adreces[index].ip == NULL) {
        free(navegant->adreces[index].name);
        free(navegant->adreces[index].ip);
        UTILS_printF(1, MSG_SENSE_MEMORIA);
        return ERR_CODI;
    }

    navegant->adreces[index].port = port;
    navegant->n_adreces++;

    return 0;
}
