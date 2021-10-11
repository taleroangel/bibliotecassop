/**
 * @file server.h
 * @authors  Ángel David Talero
 *          Juan Esteban Urquijo
 *          Humberto Rueda Cataño
 * @brief Proceso receptor de peticiones
 * @copyright 2021
 * Pontificia Universidad Javeriana
 * Facultad de Ingeniería
 * Bogotá D.C - Colombia
 */

#ifndef __SERVER_H__
#define __SERVER_H__

#include <stdbool.h>
#include "common.h"
#include "data.h"

/* ----------------------------- Definiciones ----------------------------- */

/* ------------------------------ Estructuras ------------------------------ */

/**
 * @struct client_t
 * @brief Estructura con la infroamción de un cliente
 * 
 */
typedef struct
{
    int pipe;                 /**< Pipe FD*/
    pid_t clientPID;          /**< Client PID*/
    char pipeNom[TAM_STRING]; /**< Pipe name*/
} client_t;

/**
 * @struct client_list
 * @brief Arreglo dinámico de clientes conectados
 * 
 */
struct client_list
{
    int nClients;          /**< Número de clientes en el arreglo*/
    client_t *clientArray; /**< Arreglo de clientes conectados*/
};

/* ------------------------ Prototipos de funciones ------------------------ */
/*
 - NOTA:
 - Se declaran estáticas algunas funciones para hacer privada la declaración de
 las mismas y evitar que el uso incorrecto de este header provoque problemas en 
 las implementaciones tanto para servidor como para cliente
*/

/* ------------------------- Manejo de argumentos ------------------------- */

/**
 * @brief Mostrar el uso correcto del ejecutable con sus flags
 * 
 */
void mostrarUso(void);

/**
 * @brief Verificar y manipular los argumentos recibidos
 * 
 * @param argc Numero de argumentos
 * @param argv Vector con los argumentos
 * @param pipeNom RETORNA: nombre del pipe
 * @param fileIn RETORNA: Nombre del archivo de entrada
 * @param fileOut RETORNA: Nombre del archivo de salida
 */
static void manejarArgumentos(
    int argc,
    char *argv[],
    char *pipeNom,
    char *fileIn,
    char *fileOut);

/* ----------------------- Protocolos de comunicación ----------------------- */

/**
 * @brief Iniciar la comunicación, permitir a los cliente conectarse
 * Descripción del proceso en README.md
 * 
 * @param pipeCTE_SER Nombre del pipe (Cliente->Servidor)
 * @return fd del Pipe (Cliente-Servidor)
 */
static int startCommunication(const char *pipeCTE_SER);

/**
 * @brief Conectar un cliente a la lista
 * 
 * @param clients Apuntador a la lista de clientes
 * @param package Copia del paquete recibido por el pipe
 * @return int Exit error code or SUCCESS_GENERIC
 */
int connectClient(struct client_list *clients, data_t package);

/**
 * @brief Desconectar un cliente de la lista
 * 
 * @param clients Apuntador a la lista de clientes
 * @param package Copia del paquete recibido por el pipe
 * @return int Exit error code or SUCCESS_GENERIC
 */
int disconnectClient(struct client_list *clients, data_t package);

/**
 * @brief Interpretar una señal
 * 
 * @param package Paquete con la señal
 * @return int (-1) si hay algún error
 */
int interpretSignal(struct client_list *clients, data_t package);

/**
 * @brief Generate a SIGNAL package as a reponse to the client
 * 
 * @param dest Destination Client PID
 * @param code Signal code
 * @param buffer Buffer [OPTIONAL], NULL if not required
 * @return data_t new package
 */
data_t generateReponse(pid_t dest, int code, char *buffer);

/* --------------------------- Manejo de clientes --------------------------- */

/**
 * @brief Crear un Cliente
 * 
 * @param pipefd FD del pipe del cliente
 * @param clientpid PID del cliente
 * @param pipenom Nombre del pipe del cliente
 * @return client_t Estructura con el cliente
 */
client_t createClient(int pipefd, pid_t clientpid, char *pipenom);

/**
 * @brief Guardar un cliente en el arreglo
 * 
 * @param clients Apuntador a arreglo de clientes
 * @param client Cliente a guardar
 * @return non 0 if error
 */
int storeClient(struct client_list *clients, client_t client);

/**
 * @brief Remover un cliente dado su pid
 * 
 * @param clients Apuntador a arreglo de clientes
 * @param clientToRemove Cliente a guardar
 * @return non 0 if error
 */
int removeClient(struct client_list *clients, pid_t clientToRemove);

/**
 * @brief Buscar un cliente dado su PID
 * 
 * @param client_list Lista con los clientes
 * @param client PID del cliente
 * @return int Retorna el FD del pipe (-1 si no existe)
 */
int searchClient(struct client_list *clients, pid_t client);

/**
 * @brief Manejar una solicitud de libro
 * 
 * @param clients Lista de los clientes
 * @param package Paquete
 * @param ejemplar Arreglo con los libros de la BD
 * @return int Error o 0 si exitoso
 */
int manejarLibros(
    struct client_list *clients,
    data_t package,
    struct ejemplar ejemplar[]);

#endif // __SERVER_H__