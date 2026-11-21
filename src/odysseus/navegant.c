/***********************************************
*
* @Proposit: Implementacio de l'estat del navegant Odysseus.
* @Autor/s: Arnau Ricart i Gerard Mas
* @Data creacio: 29/10/2026
* @Data ultima modificacio: 19/11/2026
*
************************************************/

//Include del fitxer .h del mateix modul
#include "navegant.h"

//Procediments i funcions

int NAVEGANT_inicialitzarNavegant (Navegant *navegant) {
    navegant->estat = ESTAT_ITHACA;
    navegant->fd_connexio = ERR_CODI;
    navegant->fd_avis = ERR_CODI;
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

void NAVEGANT_alliberarAliments (Food *aliments, int n_aliments) {
    int i;

    for (i = 0; i < n_aliments; i++) {
        free(aliments[i].name);
    }
    free(aliments);
}

int NAVEGANT_escriureLinia (int fd, char *linia) {
    int llargada;
    int resultat = ERR_CODI;

    if (linia != NULL) {
        llargada = strlen(linia);
        if (write(fd, linia, llargada) == llargada) {
            resultat = 0;
        }
        free(linia);
    }

    return resultat;
}

int NAVEGANT_escriureViatge (int fd, Navegant *navegant) {
    char *linia = NULL;

    if (navegant->te_viatge != AMB_VIATGE || navegant->viatge.file == NULL) {
        return NAVEGANT_escriureLinia(fd, UTILS_copiarText(TEXT_ESTAT_SENSE_VIATGE));
    }

    if (asprintf(&linia, FORMAT_ESTAT_VIATGE, AMB_VIATGE, navegant->viatge.id, navegant->viatge.object, navegant->viatge.file, navegant->viatge.destination, navegant->viatge.reward, navegant->viatge_lliurat) < 0) {
        linia = NULL;
    }

    return NAVEGANT_escriureLinia(fd, linia);
}

int NAVEGANT_escriureRutes (int fd, Navegant *navegant) {
    char *linia = NULL;
    int n_connexions = 0;
    int resultat;
    int i;
    int j;

    if (asprintf(&linia, FORMAT_ESTAT_TEXT, navegant->ubicacio) < 0) {
        linia = NULL;
    }
    resultat = NAVEGANT_escriureLinia(fd, linia);

    if (resultat == 0) {
        if (asprintf(&linia, FORMAT_ESTAT_NOMBRE, navegant->n_adreces) < 0) {
            linia = NULL;
        }
        resultat = NAVEGANT_escriureLinia(fd, linia);
    }

    for (i = 0; i < navegant->n_adreces && resultat == 0; i++) {
        if (asprintf(&linia, FORMAT_ESTAT_ADRECA, navegant->adreces[i].name, navegant->adreces[i].ip, navegant->adreces[i].port) < 0) {
            linia = NULL;
        }
        resultat = NAVEGANT_escriureLinia(fd, linia);
    }

    for (i = 0; i < navegant->n_illes; i++) {
        n_connexions = n_connexions + navegant->mapa[i].known_island_count;
    }

    if (resultat == 0) {
        if (asprintf(&linia, FORMAT_ESTAT_NOMBRE, n_connexions) < 0) {
            linia = NULL;
        }
        resultat = NAVEGANT_escriureLinia(fd, linia);
    }

    for (i = 0; i < navegant->n_illes && resultat == 0; i++) {
        for (j = 0; j < navegant->mapa[i].known_island_count && resultat == 0; j++) {
            if (asprintf(&linia, FORMAT_ESTAT_CONNEXIO, navegant->mapa[i].name, navegant->mapa[i].known_islands[j]) < 0) {
                linia = NULL;
            }
            resultat = NAVEGANT_escriureLinia(fd, linia);
        }
    }

    return resultat;
}

int NAVEGANT_guardarEstat (Navegant *navegant) {
    char *ruta = NULL;
    char *linia = NULL;
    int fd;
    int i;
    int resultat;

    if (asprintf(&ruta, FORMAT_RUTA_ESTAT, navegant->config.folder) < 0) {
        UTILS_printF(1, MSG_SENSE_MEMORIA);
        return ERR_CODI;
    }
    fd = open(ruta, O_WRONLY | O_CREAT | O_TRUNC, PERMISOS_FITXER);
    free(ruta);
    if (fd < 0) {
        UTILS_printF(1, MSG_ERROR_GUARDAR_ESTAT);
        return ERR_CODI;
    }

    if (asprintf(&linia, FORMAT_ESTAT_RECURSOS, navegant->config.gold, navegant->config.n_foods) < 0) {
        linia = NULL;
    }
    resultat = NAVEGANT_escriureLinia(fd, linia);

    for (i = 0; i < navegant->config.n_foods && resultat == 0; i++) {
        if (asprintf(&linia, FORMAT_ESTAT_ALIMENT, navegant->config.foods[i].name, navegant->config.foods[i].amount) < 0) {
            linia = NULL;
        }
        resultat = NAVEGANT_escriureLinia(fd, linia);
    }

    if (resultat == 0) {
        resultat = NAVEGANT_escriureViatge(fd, navegant);
    }

    if (resultat == 0) {
        resultat = NAVEGANT_escriureRutes(fd, navegant);
    }
    close(fd);

    if (resultat == ERR_CODI) {
        UTILS_printF(1, MSG_ERROR_GUARDAR_ESTAT);
    }

    return resultat;
}

void NAVEGANT_reiniciarEstat (Navegant *navegant) {
    char *ruta = NULL;
    int fd;

    if (asprintf(&ruta, FORMAT_RUTA_ESTAT, navegant->config.folder) < 0) {
        UTILS_printF(1, MSG_SENSE_MEMORIA);
        return;
    }

    fd = open(ruta, O_WRONLY | O_CREAT | O_TRUNC, PERMISOS_FITXER);
    free(ruta);

    if (fd < 0) {
        UTILS_printF(1, MSG_ERROR_GUARDAR_ESTAT);
    } else {
        close(fd);
    }
}

int NAVEGANT_carregarAliments (Navegant *navegant, int fd, int n_aliments) {
    Food *aliments = NULL;
    char *linia = NULL;
    char **camps = NULL;
    int n_camps = 0;
    int i = 0;
    int correcte = 1;

    if (n_aliments > 0) {
        aliments = malloc(sizeof(Food) * n_aliments);
        if (aliments == NULL) {
            UTILS_printF(1, MSG_SENSE_MEMORIA);
            return ERR_CODI;
        }
    }

    while (i < n_aliments && correcte == 1) {
        correcte = 0;
        linia = UTILS_readUntil(fd, '\n');
        camps = UTILS_splitString(linia, '&', &n_camps);
        if (n_camps == N_CAMPS_ESTAT_ALIMENT && UTILS_esNumeric(camps[1]) == TEXT_NUMERIC) {
            aliments[i].name = UTILS_copiarText(camps[0]);
            aliments[i].amount = atoi(camps[1]);
            if (aliments[i].name != NULL) {
                correcte = 1;
                i++;
            }
        }
        UTILS_freeTokens(camps, n_camps);
        free(linia);
    }

    if (correcte == 0) {
        NAVEGANT_alliberarAliments(aliments, i);
        return ERR_CODI;
    }

    NAVEGANT_alliberarAliments(navegant->config.foods, navegant->config.n_foods);
    navegant->config.foods = aliments;
    navegant->config.n_foods = n_aliments;

    return 0;
}

void NAVEGANT_carregarViatge (Navegant *navegant, int fd) {
    char *linia = NULL;
    char **camps = NULL;
    int n_camps = 0;

    linia = UTILS_readUntil(fd, '\n');
    camps = UTILS_splitString(linia, '&', &n_camps);

    if (n_camps == N_CAMPS_ESTAT_VIATGE && atoi(camps[0]) == AMB_VIATGE && UTILS_esNumeric(camps[1]) == TEXT_NUMERIC && UTILS_esNumeric(camps[5]) == TEXT_NUMERIC && strlen(camps[3]) > strlen(navegant->config.folder) + 1) {
        navegant->te_viatge = AMB_VIATGE;
        navegant->viatge.id = atoi(camps[1]);
        navegant->viatge.object = UTILS_copiarText(camps[2]);
        navegant->viatge.file = UTILS_copiarText(camps[3]);
        navegant->viatge.destination = UTILS_copiarText(camps[4]);
        navegant->viatge.reward = atoi(camps[5]);

        if (atoi(camps[6]) == VIATGE_LLIURAT) {
            if (asprintf(&navegant->fitxer_segellat, FORMAT_FITXER_RUTA, navegant->config.folder, TEXT_PREFIX_SEGELLAT, camps[3] + strlen(navegant->config.folder) + 1) < 0) {
                navegant->fitxer_segellat = NULL;
            } else {
                navegant->viatge_lliurat = VIATGE_LLIURAT;
            }
        }

        if (navegant->viatge.object == NULL || navegant->viatge.file == NULL || navegant->viatge.destination == NULL || (atoi(camps[6]) == VIATGE_LLIURAT && navegant->fitxer_segellat == NULL)) {
            UTILS_printF(1, MSG_SENSE_MEMORIA);
            NAVEGANT_esborrarViatge(navegant);
        }
    }

    UTILS_freeTokens(camps, n_camps);
    free(linia);
}

void NAVEGANT_carregarUbicacio (Navegant *navegant, int fd) {
    char *linia = NULL;

    linia = UTILS_readUntil(fd, '\n');

    if (linia != NULL && linia[0] != '\0' && strcasecmp(linia, TEXT_ITHACA) != 0) {
        if (NAVEGANT_canviarUbicacio(navegant, linia) == 0) {
            navegant->estat = ESTAT_DESCONNECTAT;
        }
    }

    free(linia);
}

int NAVEGANT_llegirNombre (int fd) {
    char *linia = NULL;
    int nombre = ERR_CODI;

    linia = UTILS_readUntil(fd, '\n');

    if (UTILS_esNumeric(linia) == TEXT_NUMERIC) {
        nombre = atoi(linia);
    }

    free(linia);

    return nombre;
}

void NAVEGANT_carregarRutes (Navegant *navegant, int fd) {
    char *linia = NULL;
    char **camps = NULL;
    int n_camps = 0;
    int n_linies;
    int correcte = 1;
    int i;

    n_linies = NAVEGANT_llegirNombre(fd);

    for (i = 0; i < n_linies && correcte == 1; i++) {
        correcte = 0;
        linia = UTILS_readUntil(fd, '\n');
        camps = UTILS_splitString(linia, '&', &n_camps);
        if (n_camps == N_CAMPS_ESTAT_ADRECA && UTILS_esNumeric(camps[2]) == TEXT_NUMERIC && NAVEGANT_afegirAdreca(navegant, camps[0], camps[1], atoi(camps[2])) == 0) {
            correcte = 1;
        }
        UTILS_freeTokens(camps, n_camps);
        free(linia);
    }

    if (correcte == 0) {
        return;
    }

    n_linies = NAVEGANT_llegirNombre(fd);

    for (i = 0; i < n_linies && correcte == 1; i++) {
        correcte = 0;
        linia = UTILS_readUntil(fd, '\n');
        camps = UTILS_splitString(linia, '&', &n_camps);
        if (n_camps == N_CAMPS_ESTAT_CONNEXIO && NAVEGANT_afegirRuta(navegant, camps[0], camps[1]) == 0) {
            correcte = 1;
        }
        UTILS_freeTokens(camps, n_camps);
        free(linia);
    }
}

int NAVEGANT_carregarEstat (Navegant *navegant) {
    char *ruta = NULL;
    char *linia = NULL;
    char **camps = NULL;
    int n_camps = 0;
    int fd;
    int resultat = ERR_CODI;

    if (asprintf(&ruta, FORMAT_RUTA_ESTAT, navegant->config.folder) < 0) {
        UTILS_printF(1, MSG_SENSE_MEMORIA);
        return ERR_CODI;
    }
    fd = open(ruta, O_RDONLY);
    free(ruta);
    if (fd < 0) {
        return ERR_CODI;
    }

    linia = UTILS_readUntil(fd, '\n');
    camps = UTILS_splitString(linia, '&', &n_camps);

    if (n_camps == N_CAMPS_ESTAT_RECURSOS && UTILS_esNumeric(camps[0]) == TEXT_NUMERIC && UTILS_esNumeric(camps[1]) == TEXT_NUMERIC && NAVEGANT_carregarAliments(navegant, fd, atoi(camps[1])) == 0) {
        navegant->config.gold = atoi(camps[0]);
        NAVEGANT_carregarViatge(navegant, fd);
        NAVEGANT_carregarUbicacio(navegant, fd);
        NAVEGANT_carregarRutes(navegant, fd);
        resultat = 0;
    }

    UTILS_freeTokens(camps, n_camps);
    free(linia);
    close(fd);

    return resultat;
}

int NAVEGANT_senseSortida (Navegant *navegant) {
    int resultat = AMB_SORTIDA;

    if (navegant->config.gold < OR_MINIM_SORTIDA && NAVEGANT_totalProvisions(navegant) == 0 && navegant->viatge_lliurat != VIATGE_LLIURAT && (navegant->te_viatge != AMB_VIATGE || (navegant->estat != ESTAT_ATRACAT && navegant->estat != ESTAT_DESCONNECTAT) || strcasecmp(navegant->ubicacio, navegant->viatge.destination) != 0)) {
        resultat = SENSE_SORTIDA;
    }

    return resultat;
}
