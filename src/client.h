/**
 * @file client.h
 * @author  Ángel David Talero
 *          Juan Esteban Urquijo
 *          Humberto Rueda Cataño
 * @brief Proceso solicitante
 * @copyright 2021
 * Pontificia Universidad Javeriana
 * Facultad de Ingeniería
 * Bogotá D.C - Colombia
 */

#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <stdbool.h>
#include "data.h"

/* ----------------------------- Definiciones ----------------------------- */

#define PIPE_NOM_CTE "pipeCliente_" // Nombre con el cual crear los pipes de cliente

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
 * @param fileNom RETORNA: nombre del archivo
 * @return true Se utilizó un archivo
 * @return false No se utilizó un archivo, por lo tanto ignorar el contenido de fileNom
 */
static bool manejarArgumentos(
    int argc,
    char *argv[],
    char *pipeNom,
    char *fileNom);

/* ----------------------- Protocolos de comunicación ----------------------- */

/**
 * @brief Iniciar la comunicación con el servidor
 * Descripción del proceso en README.md
 * 
 * @param pipeCTE_SER Nombre del pipe (Cliente->Servidor)
 * @param pipeSER_CTE RETORNA: nombre escogido para el pipe (Servidor->Cliente)
 * @param pipe pipe[0] tiene el pipe del pipe (Cliente -> Servidor) y
 *          pipe[1] tiene el pipe del pipe (Servidor -> Cliente)
 * Use las macros WRITE y READ con pipe
 * EJ: para escribir en el pipe se utilizar pipe[WRITE]
 */
static void startCommunication(
    const char *pipeCTE_SER,
    char *pipeSER_CTE,
    int *pipe);

static void stopCommunication(int *pipe, char *pipeSER_CTE);

/**
 * @brief Generate a SIGNAL package
 * 
 * @param src Source Client PID
 * @param code Signal code
 * @param buffer Buffer [OPTIONAL], NULL if not required
 * @return data_t new package
 */
data_t generateSignal(pid_t src, int code, char *buffer);

/* ----------------------- Funciones para los libros ----------------------- */

/**
 * @brief Función que se encarga de pedir prestado un libro al servidor
 * 
 * @param pipes Arreglo con los pipes
 * @param nombreLibro Nombre del libro
 * @param ISBN Su ISBN
 * @return int Código de error o SUCCESS_GENERIC (0) si éxito
 */
int prestarLibro(int *pipes, const char *nombreLibro, int ISBN);

/**
 * @brief Función que se encarga de pedir devolver un libro al servidor
 * 
 * @param pipes Arreglo con los pipes
 * @param nombreLibro Nombre del libro
 * @param ISBN Su ISBN
 * @param ejemplar Número de ejemplar
 * @return int Código de error o SUCCESS_GENERIC (0) si éxito
 */
int devolverLibro(
    int *pipes,
    const char *nombreLibro,
    int ISBN,
    int ejemplar);

/**
 * @brief Función que se encarga de pedir renovar un libro al servidor
 * 
 * @param pipes Arreglo con los pipes
 * @param nombreLibro Nombre del libro
 * @param ISBN Su ISBN
 * @param ejemplar Número de ejemplar
 * @return int Código de error o SUCCESS_GENERIC (0) si éxito
 */
int renovarLibro(
    int *pipes,
    const char *nombreLibro,
    int ISBN,
    int ejemplar);

/**
 * @brief Pedirle al servidor la información de un libro en específico
 * 
 * @param pipes Pipes de comunicación
 * @param nombre Nombre del libro
 * @param ISBN ISBN del libro
 * @return struct ejemplar Estructura que contiene al libro, en caso de error
 * la petición del libro es BUSCAR, cualquier otro tipo de petición es éxito
 */
struct ejemplar buscarLibro(int *pipes, const char *nombre, int ISBN);

#endif // __CLIENT_H__