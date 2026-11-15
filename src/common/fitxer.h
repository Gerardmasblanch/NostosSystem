/***********************************************
*
* @Proposit: Operacions amb fitxers per a les transferencies del sistema.
* @Autor/s: Arnau Ricart i Gerard Mas
* @Data creacio: 14/11/2026
* @Data ultima modificacio: 15/11/2026
*
************************************************/

//Define Guards
#ifndef _FITXER_H
#define _FITXER_H

//Llibreries del sistema
#include <sys/wait.h>

//Llibreries propies
#include "trama.h"

//Constants
#define FITXER_OK          0
#define FITXER_ERROR      -1
#define FITXER_ERROR_MIDA -2
#define FITXER_ERROR_MD5  -3
#define FITXER_REBUTJAT   -4

#define FITXER_NOM_NO_VALID 0
#define FITXER_NOM_VALID    1

#define N_CAMPS_FITXER  4
#define MIDA_MD5        32
#define PERMISOS_FITXER 0644
#define COMANDA_MD5     "md5sum"

#define FORMAT_FITXER_CAPCALERA "%s&%s&%d&%s"
#define FORMAT_FITXER_RUTA      "%s/%s%s"

#define TEXT_FITXER_OBJECT      "OBJECT"
#define TEXT_FITXER_SIGNED      "SIGNED"
#define TEXT_FITXER_READY       "READY"
#define TEXT_FITXER_OK          "OK"
#define TEXT_FITXER_ERROR_MIDA  "SIZE_ERROR"
#define TEXT_FITXER_ERROR_MD5   "MD5_ERROR"
#define TEXT_FITXER_ERROR_CREAR "FILE_ERROR"

#define TEXT_FITXER_INVALID_TYPE  "INVALID_TYPE"
#define TEXT_FITXER_INVALID_FLAGS "INVALID_FLAGS"
#define TEXT_FITXER_INVALID_DATA  "INVALID_DATA"

#define MSG_FITXER_ERROR_OBRIR       "Error: no s'ha pogut obrir el fitxer.\n"
#define MSG_FITXER_ERROR_MIDA        "Error: no s'ha pogut obtenir la mida del fitxer.\n"
#define MSG_FITXER_ERROR_MD5         "Error: no s'ha pogut calcular l'MD5 del fitxer.\n"
#define MSG_FITXER_ERROR_LLEGIR      "Error: no s'ha pogut llegir el bloc complet del fitxer.\n"
#define MSG_FITXER_ERROR_ESCRIURE    "Error: no s'ha pogut escriure el bloc al fitxer.\n"
#define MSG_FITXER_ERROR_ENVIAR      "Error: no s'ha pogut enviar la trama del fitxer.\n"
#define MSG_FITXER_ERROR_REBRE       "Error: no s'ha pogut rebre la trama del fitxer.\n"
#define MSG_FITXER_CAPCALERA_LLARGA  "Error: la capcalera del fitxer supera els 253 bytes.\n"
#define MSG_FITXER_MIDA_INVALIDA     "Error: la mida del fitxer no pot ser negativa.\n"
#define MSG_FITXER_RESPOSTA_INVALIDA "Error: resposta de transferencia no valida.\n"
#define MSG_FITXER_REBUTJAT          "Error: el receptor ha rebutjat la transferencia.\n"
#define MSG_FITXER_MIDA_DIFERENT     "Error: el receptor ha detectat una mida incorrecta.\n"
#define MSG_FITXER_MD5_DIFERENT      "Error: el receptor ha detectat un MD5 diferent.\n"

//Procediments i funcions

/***********************************************
*
* @Finalitat: Obte la mida en bytes d'un fitxer.
* @Parametres:  in: path = ruta del fitxer.
* @Retorn: Mida en bytes, o FITXER_ERROR si hi ha un error.
*
************************************************/
int FITXER_midaFitxer (char *path);

/***********************************************
*
* @Finalitat: Executa md5sum i obte el resum MD5 d'un fitxer.
* @Parametres:  in: path = ruta del fitxer.
*               out: md5 = buffer amb espai per a MIDA_MD5 + 1 caracters.
* @Retorn: FITXER_OK si s'ha obtingut l'MD5, FITXER_ERROR si no.
*
************************************************/
int FITXER_calcularMD5 (char *path, char *md5);

/***********************************************
*
* @Finalitat: Envia un fitxer seguint el protocol de transferencia.
* @Parametres:  in: fd = socket connectat amb el receptor.
*               in: path = ruta local del fitxer que s'envia.
*               in: nom_fitxer = nom del fitxer comunicat al receptor.
*               in: etiqueta = OBJECT, DELIVERY, SIGNED o CLAIM.
*               in: es_resposta = 1 si es una resposta, 0 si es una peticio.
* @Retorn: FITXER_OK si la transferencia es confirma correctament;
*          FITXER_ERROR si hi ha un error local o de comunicacio;
*          FITXER_ERROR_MIDA o FITXER_ERROR_MD5 segons l'ACK rebut;
*          FITXER_REBUTJAT si el receptor rebutja la transferencia.
*          Si retorna un error, el cridant ha de tancar la connexio, perque poden quedar trames a mitges al socket.
*
************************************************/
int FITXER_enviar (int fd, char *path, char *nom_fitxer, char *etiqueta, int es_resposta);

/***********************************************
*
* @Finalitat: Construeix i envia la capcalera d'una transferencia.
* @Parametres:  in: fd = socket connectat amb el receptor.
*               in: nom_fitxer = nom comunicat al receptor.
*               in: etiqueta = OBJECT, DELIVERY, SIGNED o CLAIM.
*               in: mida = mida del fitxer en bytes.
*               in: md5 = resum MD5 del fitxer.
*               in: es_resposta = 1 per resposta, 0 per peticio.
* @Retorn: FITXER_OK si s'envia correctament, FITXER_ERROR si no.
*
************************************************/
int FITXER_enviarCapcalera (int fd, char *nom_fitxer, char *etiqueta, int mida, char *md5, int es_resposta);

/***********************************************
*
* @Finalitat: Espera la resposta READY del receptor d'un fitxer. Si la trama rebuda no es valida i no es un NACK, respon amb un NACK amb el motiu.
* @Parametres:  in: fd = socket connectat amb el receptor.
* @Retorn: FITXER_OK si el receptor esta preparat;
*          FITXER_REBUTJAT si rebutja la transferencia;
*          FITXER_ERROR si falla la recepcio o la resposta no es valida.
*
************************************************/
int FITXER_esperarReady (int fd);

/***********************************************
*
* @Finalitat: Llegeix i envia els blocs de dades d'un fitxer.
* @Parametres:  in: fd = socket connectat amb el receptor.
*               in: fd_fitxer = fitxer obert per lectura i situat a l'inici.
*               in: mida = nombre total de bytes que s'han d'enviar.
*               in: es_resposta = 1 per resposta, 0 per peticio.
* @Retorn: FITXER_OK si envia tots els blocs, FITXER_ERROR si no.
*
************************************************/
int FITXER_enviarBlocs (int fd, int fd_fitxer, int mida, int es_resposta);

/***********************************************
*
* @Finalitat: Espera la confirmacio final d'una transferencia. Si la trama rebuda no es valida i no es un NACK, respon amb un NACK amb el motiu.
* @Parametres:  in: fd = socket connectat amb el receptor.
* @Retorn: FITXER_OK si el receptor confirma la transferencia;
*          FITXER_ERROR_MIDA si informa d'una mida incorrecta;
*          FITXER_ERROR_MD5 si informa d'un MD5 diferent;
*          FITXER_ERROR si falla la recepcio o la resposta no es valida.
*
************************************************/
int FITXER_esperarAck (int fd);

/***********************************************
*
* @Finalitat: Informa per pantalla d'una trama no valida i envia un NACK amb el motiu.
* @Parametres:  in: fd = socket on s'envia el NACK.
*               in: motiu = INVALID_TYPE, INVALID_FLAGS, INVALID_DATA o INVALID_LENGTH.
* @Retorn: ----.
*
************************************************/
void FITXER_enviarNack (int fd, char *motiu);

/***********************************************
*
* @Finalitat: Rep un fitxer seguint el protocol de transferencia.
* @Parametres:  in: fd = socket connectat amb l'emissor.
*               in: carpeta = carpeta on es desara el fitxer.
*               in: etiqueta_esperada = OBJECT, DELIVERY, SIGNED o CLAIM.
*               in: prefix = text afegit davant del nom del fitxer.
*               out: path_rebut = ruta reservada amb malloc si te exit;
*                                 NULL si falla. El cridant l'ha d'alliberar.
* @Retorn: FITXER_OK si rep i verifica el fitxer;
*          FITXER_ERROR_MIDA si la mida es incorrecta;
*          FITXER_ERROR_MD5 si l'MD5 es diferent;
*          FITXER_ERROR si hi ha un altre error.
*          Si retorna un error, el cridant ha de tancar la connexio, perque poden quedar trames a mitges al socket.
*
************************************************/
int FITXER_rebre (int fd, char *carpeta, char *etiqueta_esperada,  char *prefix, char **path_rebut);

/***********************************************
*
* @Finalitat: Separa i valida els quatre camps d'una capcalera.
* @Parametres:  in: dades = text de la capcalera rebuda.
*               in: etiqueta_esperada = etiqueta que ha de contenir.
*               out: mida = mida del fitxer convertida a enter.
* @Retorn: Array de quatre cadenes si es valid, NULL si falla.
*          El cridant l'ha d'alliberar amb UTILS_freeTokens.
*
************************************************/
char **FITXER_separarCapcalera (char *dades, char *etiqueta_esperada, int *mida);

/***********************************************
*
* @Finalitat: Comprova que un nom de fitxer rebut es pugui fer servir.
* @Parametres:  in: nom = nom del fitxer rebut a la capcalera.
* @Retorn: FITXER_NOM_VALID si no es buit, ni "." ni "..", i no conte
*          '/' ni '\'; FITXER_NOM_NO_VALID en cas contrari.
*
************************************************/
int FITXER_nomFitxerValid (char *nom);

/***********************************************
*
* @Finalitat: Rep i valida la capcalera d'una transferencia. Si no es valida, respon NACK si el tipus no es 0x20, o READY amb 0xC0 si ho es.
* @Parametres:  in: fd = socket connectat amb l'emissor.
*               in: etiqueta_esperada = etiqueta que s'espera rebre.
*               out: mida = mida del fitxer en bytes.
*               out: flags_dades = flags esperats als blocs de dades.
* @Retorn: Array de quatre cadenes si es valid, NULL si falla.
*          El cridant l'ha d'alliberar amb UTILS_freeTokens.
*
************************************************/
char **FITXER_rebreCapcalera (int fd, char *etiqueta_esperada, int *mida, int *flags_dades);

/***********************************************
*
* @Finalitat: Prepara el fitxer de desti i envia la resposta READY.
* @Parametres:  in: fd = socket connectat amb l'emissor.
*               in: carpeta = carpeta de desti, que ha d'existir.
*               in: prefix = text afegit davant del nom.
*               in: nom_fitxer = nom rebut a la capcalera.
*               out: path_rebut = ruta reservada, o NULL si falla.
* @Retorn: Descriptor del fitxer obert, o FITXER_ERROR si falla.
*          El cridant ha de tancar el fitxer i alliberar la ruta.
*
************************************************/
int FITXER_prepararRecepcio (int fd, char *carpeta, char *prefix, char *nom_fitxer, char **path_rebut);

/***********************************************
*
* @Finalitat: Rep els blocs i els escriu al fitxer de desti.
* @Parametres:  in: fd = socket connectat amb l'emissor.
*               in: fd_fitxer = fitxer de desti obert per escriure.
*               in: mida = mida anunciada a la capcalera.
*               in: flags_dades = flags esperats als blocs.
* @Retorn: FITXER_OK si rep i escriu els blocs, FITXER_ERROR si falla.
*          La comprovacio de la mida final es fa posteriorment.
*
************************************************/
int FITXER_rebreBlocs (int fd, int fd_fitxer, int mida, int flags_dades);

/***********************************************
*
* @Finalitat: Verifica el fitxer rebut i envia l'ACK final.
* @Parametres:  in: fd = socket connectat amb l'emissor.
*               in: path = ruta del fitxer, amb l'escriptura finalitzada.
*               in: mida_esperada = mida anunciada a la capcalera.
*               in: md5_esperat = MD5 anunciat a la capcalera.
* @Retorn: FITXER_OK si es correcte;
*          FITXER_ERROR_MIDA o FITXER_ERROR_MD5 si no coincideixen;
*          FITXER_ERROR si hi ha un error local o de comunicacio.
*
************************************************/
int FITXER_verificarRecepcio (int fd, char *path, int mida_esperada, char *md5_esperat);

#endif