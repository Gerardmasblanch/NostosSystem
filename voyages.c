/***********************************************
*
* @Proposit: Implementacio de la gestio dels clients i dels viatges del servidor Ithaca.
* @Autor/s: Arnau Ricart i Gerard Mas
* @Data creacio: 29/10/2026
* @Data ultima modificacio: 29/10/2026
*
************************************************/

#include "voyages.h"

/***********************************************
*
* @Finalitat: Inicialitzar el servidor sense clients connectats.
* @Parametres: out: servidor = estructura del servidor a inicialitzar.
* @Retorn: ----.
*
************************************************/
void inicialitzarServidorIthaca (ServidorIthaca *servidor) {
    servidor->clients = NULL;
    servidor->n_clients = 0;
}

/***********************************************
*
* @Finalitat: Acceptar una connexio i afegir el client al servidor.
* @Parametres: in/out: servidor = servidor que guardara el client.
*              in: fd_server = socket que escolta connexions.
* @Retorn: 0 si s'ha afegit el client, ERR_CODI si hi ha un error.
*
************************************************/
int acceptarClientIthaca (ServidorIthaca *servidor, int fd_server) {
    ClientIthaca *nous_clients = NULL;
    int fd_client;

    fd_client = accept(fd_server, NULL, NULL);
    if (fd_client == ERR_CODI) {
        printF(1, MSG_ERROR_ACCEPT);
        return ERR_CODI;
    }

    if (fd_client >= FD_SETSIZE) {
        printF(1, MSG_ERROR_DESCRIPTOR);
        close(fd_client);
        return ERR_CODI;
    }

    nous_clients = realloc(servidor->clients, sizeof(ClientIthaca) * (servidor->n_clients + 1));

    if (nous_clients == NULL) {
        printF(1, MSG_SENSE_MEMORIA);
        close(fd_client);
        return ERR_CODI;
    }

    servidor->clients = nous_clients;
    servidor->clients[servidor->n_clients].fd = fd_client;
    servidor->clients[servidor->n_clients].nom = NULL;
    servidor->n_clients++;

    return 0;
}

/***********************************************
*
* @Finalitat: Preparar el conjunt de sockets que vigilara select.
* @Parametres: out: conjunt = conjunt de descriptors a preparar.
*              in: fd_server = socket que escolta connexions.
*              in: servidor = servidor amb els clients connectats.
* @Retorn: El descriptor mes gran del conjunt.
*
************************************************/
int prepararSetIthaca (fd_set *conjunt, int fd_server, ServidorIthaca *servidor) {
    int i;
    int max_fd = fd_server;

    FD_ZERO(conjunt);
    FD_SET(fd_server, conjunt);

    for (i = 0; i < servidor->n_clients; i++) {
        FD_SET(servidor->clients[i].fd, conjunt);

        if (servidor->clients[i].fd > max_fd) {
            max_fd = servidor->clients[i].fd;
        }
    }

    return max_fd;
}

/***********************************************
*
* @Finalitat: Tancar la connexio d'un client i treure'l de la llista.
* @Parametres: in/out: servidor = servidor amb els clients connectats.
*              in: index = posicio del client que s'ha de treure.
* @Retorn: ----.
*
************************************************/
void treureClientIthaca (ServidorIthaca *servidor, int index) {
    int i;

    close(servidor->clients[index].fd);
    free(servidor->clients[index].nom);

    for (i = index; i < servidor->n_clients - 1; i++) {
        servidor->clients[i] = servidor->clients[i + 1];
    }

    servidor->n_clients--;
}

/***********************************************
*
* @Finalitat: Tancar les connexions dels clients i alliberar la seva llista.
* @Parametres: in/out: servidor = servidor que s'ha d'alliberar.
* @Retorn: ----.
*
************************************************/
void alliberarServidorIthaca (ServidorIthaca *servidor) {
    
    while (servidor->n_clients > 0) {
        treureClientIthaca(servidor, servidor->n_clients - 1);
    }

    free(servidor->clients);
    servidor->clients = NULL;
}

/***********************************************
*
* @Finalitat: Identificar un client amb el nom rebut i confirmar la connexio.
* @Parametres: in/out: servidor = servidor amb els clients connectats.
*              in: index = posicio del client que s'identifica.
*              in: dades = nom rebut al camp DATA de la trama.
* @Retorn: TRAMA_OK si s'ha enviat la resposta, TRAMA_ERROR si falla.
*
************************************************/
int atendreConnectIthaca (ServidorIthaca *servidor, int index, char *dades) {
    ClientIthaca *client = NULL;
    char *missatge = NULL;
    int resultat;

    client = &servidor->clients[index];

    if (client->nom != NULL) {
        return enviarTrama(client->fd, TIPUS_CONNECT, FLAG_ERROR, TEXT_ITHACA_ERROR_ESTAT);
    }

    if (strlen(dades) == 0) {
        return enviarTrama(client->fd, TIPUS_CONNECT, FLAG_ERROR, TEXT_ITHACA_ERROR_DADES);
    }

    client->nom = malloc(strlen(dades) + 1);
    if (client->nom == NULL) {
        printF(1, MSG_SENSE_MEMORIA);
        return enviarTrama(client->fd, TIPUS_CONNECT, FLAG_ERROR, TEXT_ITHACA_ERROR_SERVIDOR);
    }

    strcpy(client->nom, dades);

    resultat = enviarTrama(client->fd, TIPUS_CONNECT, FLAG_OK, TEXT_ITHACA_OK);

    if (resultat == TRAMA_OK) {
        if (asprintf(&missatge, MSG_CLIENT_CONNECTAT, client->nom) >= 0) {
            printF(1, missatge);
            free(missatge);
        } else {
            printF(1, MSG_SENSE_MEMORIA);
        }
    }

    return resultat;
}

/***********************************************
*
* @Finalitat: Llegir una trama d'un client i atendre la peticio rebuda.
* @Parametres: in/out: servidor = servidor amb els clients connectats.
*              in: index = posicio del client que te activitat.
* @Retorn: ----.
*
************************************************/
void atendreTramaIthaca (ServidorIthaca *servidor, int index) {
    char *dades = NULL;
    int tipus;
    int flags;
    int fd;
    int resultat;
    int desconnectar = 0;

    fd = servidor->clients[index].fd;

    if (rebreTrama(fd, &tipus, &flags, &dades) == TRAMA_ERROR) {
        free(dades);
        treureClientIthaca(servidor, index);
        return;
    }

    if (flags != FLAG_PETICIO) {
        resultat = enviarTrama(fd, TIPUS_NACK, FLAG_ERROR, TEXT_ITHACA_ERROR_FLAGS);
    } else if (tipus == TIPUS_CONNECT) {
        resultat = atendreConnectIthaca(servidor, index, dades);
    } else if (tipus == TIPUS_SORTIR_ITHACA) {
        if (servidor->clients[index].nom == NULL) {
            resultat = enviarTrama(fd, tipus, FLAG_ERROR, TEXT_ITHACA_ERROR_ESTAT);
        } else if (strlen(dades) != 0) {
            resultat = enviarTrama(fd, tipus, FLAG_ERROR, TEXT_ITHACA_ERROR_DADES);
        } else {
            resultat = enviarTrama(fd, tipus, FLAG_OK, TEXT_ITHACA_OK);
            desconnectar = 1;
        }
    } else {
        resultat = enviarTrama(fd, TIPUS_NACK, FLAG_ERROR, TEXT_ITHACA_ERROR_TIPUS);
    }

    free(dades);

    if (resultat == TRAMA_ERROR || desconnectar == 1) {
        treureClientIthaca(servidor, index);
    }
}

/***********************************************
*
* @Finalitat: Esperar connexions i atendre els clients amb select.
* @Parametres: in: fd_server = socket que escolta connexions.
*              in/out: servidor = servidor amb els clients connectats.
*              in: sortir = bandera activada pel handler de SIGINT.
* @Retorn: 0 si es tanca per la bandera, ERR_CODI si falla.
*
************************************************/
int executarServidorIthaca (int fd_server, ServidorIthaca *servidor, int *sortir) {
    fd_set conjunt;
    int max_fd;
    int resultat;
    int i;

    if (fd_server < 0 || fd_server >= FD_SETSIZE) {
        printF(1, MSG_ERROR_DESCRIPTOR);
        return ERR_CODI;
    }

    while (*sortir == 0) {
        max_fd = prepararSetIthaca(&conjunt, fd_server, servidor);

        resultat = select(max_fd + 1, &conjunt, NULL, NULL, NULL);

        if (resultat < 0) {
            if (*sortir == 0) {
                printF(1, MSG_ERROR_SELECT);
                return ERR_CODI;
            }
        } else if (*sortir == 0) {
            for (i = servidor->n_clients - 1; i >= 0; i--) {
                if (FD_ISSET(servidor->clients[i].fd, &conjunt)) {
                    atendreTramaIthaca(servidor, i);
                }
            }

            if (FD_ISSET(fd_server, &conjunt)) {
                acceptarClientIthaca(servidor, fd_server);
            }
        }
    }

    return 0;
}

