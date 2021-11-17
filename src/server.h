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
#include "paquet.h"
#include "buffer.h"

/* ----------------------------- Definiciones ----------------------------- */

/* ------------------------------ Estructuras ------------------------------ */

/**
 * @struct client_t
 * @brief Estructura con la información de un clinete
 * 
 */
typedef struct
{
    int pipe;                      /**< File descriptor del pipe (Servidor->Cliente) asociado*/
    pid_t clientPID;               /**< PID del cliente*/
    char pipeFilename[TAM_STRING]; /**< Nombre del pipe (Servidor->Cliente)*/

} client_t;

/**
 * @struct client_list
 * @brief Arreglo dinámico de clientes conectados
 * 
 */
struct client_list
{
    int n_clients;         /**< Número de clientes en el arreglo*/
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
 * @param pipeFilename RETORNA: nombre del pipe
 * @param fileIn RETORNA: Nombre del archivo de entrada
 * @param fileOut RETORNA: Nombre del archivo de salida
 */
static void manejarArgumentos(
    int argc,
    char *argv[],
    char *pipeNom,
    char *fileIn,
    char *fileOut);

/**
 * @brief Manejar una señal
 * 
 * @param foo Dummy
 */
void manejadorInterrupcion(int foo);

/* ----------------------- Manejo de la Base de Datos ----------------------- */
/**
 * @brief Abrir el archivo de BD y almacenar todos los libros
 * 
 * @param booksDatabase Arreglo con todos los libros
 * @param filename Nombre del archivo a leer
 * @return Cantidad de libros que se leyeron en total
 * 
 * @note El archivo se abre y se cierra en la misma función pues no tiene porqué
 * ser utilizado más adelante en el programa
 */
int leerDatabase(book_t booksDatabase[], const char filename[]);

/**
 * @brief Actualizar la información de la base de datos
 * 
 * @param filename Archivo a escribir
 * @param booksDatabase Arreglo de base de datos
 * @param tam_database Tamaño de la base de datos
 * @return Return error
 */
int actualizarDatabase(const char filename[],
                       book_t booksDatabase[],
                       int tam_database);

/**
 * @brief Mostrar la database en pantalla como alternativa
 * 
 * @param booksDatabase Arreglo de base de datos
 * @param tam_database Tamaño de la base de datos
 */
void mostrarDatabasePantalla(book_t booksDatabase[], int tam_database);

/* ----------------------- Protocolos de comunicación ----------------------- */

/**
 * @brief Iniciar la comunicación, permitir a los cliente conectarse
 * Descripción del proceso en README.md
 * 
 * @param pipeCLNT_SRVR Nombre del pipe (Cliente->Servidor)
 * @return fd del Pipe (Cliente-Servidor)
 */
static int iniciarComunicacion(const char *pipeCLNT_SRVR);

/**
 * @brief Conectar un cliente a la lista
 * 
 * @param clients Apuntador a la lista de clientes
 * @param package Copia del paquete recibido por el pipe
 * @return int Exit error code or SUCCESS_GENERIC
 */
int conectarCliente(struct client_list *clients, paquet_t package);

/**
 * @brief Desconectar un cliente de la lista
 * 
 * @param clients Apuntador a la lista de clientes
 * @param package Copia del paquete recibido por el pipe
 * @return int Exit error code or SUCCESS_GENERIC
 */
int retirarCliente(struct client_list *clients, paquet_t package);

/**
 * @brief Interpretar una señal
 * 
 * @param package Paquete con la señal
 * @return int (-1) si hay algún error
 */
int interpretarSenal(struct client_list *clients, paquet_t package);

/**
 * @brief Generar una señal como respuesta a un Cliente
 * 
 * @param dest PID del cliente destino
 * @param code Código de la señal
 * @param buffer Buffer [OPCIONAL], NULL de no necesitarse
 * @return paquet_t Nuevo paquete a enviar
 */
paquet_t generarRespuesta(pid_t dest, int code, char *buffer);

/* --------------------------- Manejo de clientes --------------------------- */

/**
 * @brief Crear un Cliente
 * 
 * @param pipefd File Descriptor del pipe (Servidor->Cliente)
 * @param clientpid PID del cliente
 * @param pipenom Nombre del pipe del cliente
 * @return client_t Estructura con el cliente
 */
client_t crearCliente(int pipefd, pid_t clientpid, char *pipenom);

/**
 * @brief Guardar un cliente en el arreglo
 * 
 * @param clients Apuntador a arreglo de clientes
 * @param client Cliente a guardar
 * @return SUCCESS_GENERIC si éxito, cualquier otro valor de lo contrario
 */
int guardarCliente(struct client_list *clients, client_t client);

/**
 * @brief Remover un cliente dado su pid
 * 
 * @param clients Apuntador a arreglo de clientes
 * @param clientToRemove Cliente a guardar
 * @return SUCCESS_GENERIC si éxito, cualquier otro valor de lo contrario
 */
int removerCliente(struct client_list *clients, pid_t clientToRemove);

/**
 * @brief Buscar un cliente dado su PID
 * 
 * @param client_list Lista con los clientes
 * @param client PID del cliente
 * @return int Retorna el FD del pipe (-1 si no existe)
 */
int buscarCliente(struct client_list *clients, pid_t client);

/**
 * @brief Manejar una solicitud de libro
 * 
 * @param clients Lista de los clientes
 * @param package Paquete recibido
 * @param ejemplar Arreglo con los libros de la BD
 * @return SUCCESS_GENERIC si éxito, cualquier otro valor de lo contrario
 */
int manejarLibros(
    struct client_list *clients,
    paquet_t package,
    book_t ejemplar[]);

/* ---------------- Manejo de concurrencia y buffer interno ---------------- */

/**
 * @struct arg_buffer
 * Argumentos de la funcoón manejador buffer
 * @param buffer Arreglo buffer
 * @param client_list Lista con los clientes
 * @param booksDatabase Base de datos con los libros
 */
struct arg_buffer
{
    buffer_t *buffer;
    struct client_list *clients;
    book_t *booksDatabase;
};

/**
 * @brief Función encargada de manejar las peticiones, se tiene que llamar desde
 * un hilo
 * 
 * @param params parametros de la función
 * @return void* Nada
 */
void *manejadorBuffer(struct arg_buffer *params);

#endif // __SERVER_H__