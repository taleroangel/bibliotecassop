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
static int iniciarComunicacion(const char *pipeCTE_SER);

/**
 * @brief Agregar un cliente a la lista
 * 
 * @param clients Apuntador a la lista de clientes
 * @param package Copia del paquete recibido por el pipe
 */
int conectarCliente(struct client_list *clients, data_t package);

#endif // __SERVER_H__