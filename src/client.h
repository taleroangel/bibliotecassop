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

/* ----------------------------- Definiciones ----------------------------- */

#define PIPE_NOM_CTE "pipeCliente_"

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

/* --------------------------- Manejo de archivo --------------------------- */

/**
 * @brief Intentar abrir un pipeNominal o Archivo en modo de sólo lectura
 * 
 * @param nombre nombre del archivo o pipe
 * @return int File Descriptor del archivo
 */
int abrirArchivo(char *nombre);

/**
 * @brief Cerrar un archivo o un pipe
 * 
 * @param fd File descriptor del archivo a cerrar
 */
void cerrarArchivo(int fd);

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
static void iniciarComunicacion(
    const char *pipeCTE_SER,
    char *pipeSER_CTE,
    int *pipe);

#endif // __CLIENT_H__