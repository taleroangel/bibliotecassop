/**
 * @file server.h
 * @author  Ángel David Talero
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
typedef struct
{
    int pipe;                 // Pipe FD
    pid_t clientPID;          // Client PID
    char pipeNom[TAM_STRING]; // Pipe name
} client_t;                   // Client structure

struct client_list // Client list structure
{
    int nClients;
    client_t *clientArray;
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
 * @return fd del pipe
 */
static int startCommunication(const char *pipeCTE_SER);

/**
 * @brief Agregar un cliente a la lista
 * 
 * @param clients Apuntador a la lista de clientes
 * @param package Copia del paquete recibido por el pipe
 * @return Exit error code or SUCCESS
 */
int connectClient(struct client_list *clients, data_t package);

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

#endif // __SERVER_H__