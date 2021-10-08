/**
 * @file common.h
 * @author  Ángel David Talero
 *          Juan Esteban Urquijo
 *          Humberto Rueda Cataño
 * @brief Macros y Typedefs compartidos entre Cliente y Servidor
 * @copyright 2021
 * Pontificia Universidad Javeriana
 * Facultad de Ingeniería
 * Bogotá D.C - Colombia
 */

#ifndef __COMMON_H__
#define __COMMON_H__

/* ------------------------------ Libraries ------------------------------ */
#include <stdint.h>

/* ----------------------------- Definiciones ----------------------------- */

#define TAM_STRING 20 // Tamaño de los Strings

#define PERMISOS_PIPE S_IRUSR | S_IWUSR // Permiso para Leer, Escribir, Ejecutar
#define INTENTOS_ESCRITURA 5            // Intentos de escritura en caso de falla
#define TIMEOUT_COMUNICACION 10         // Tiempo límite (s) para establecer comunicación

#define WRITE 0 // Definiciones para el vector fd
#define READ 1  // Definiciones para el vector fd

/* -------------------------------- Señales -------------------------------- */
/* ---------------- Señales de confirmación de comunicación ---------------- */

#define START_COM 1   // Señal para empezar comunicación
#define STOP_COM -1   // Señal para detener confirmación
#define SUCCEED_COM 2 // Señal de confirmación de comunicación
#define FAILED_COM -2 // Señal de fallo en la comunicación (TERMINACION)

/* --------------------------- Lista de errores --------------------------- */
// Errores genéricos
#define SUCCESS_GENERIC 0  // Exitoso
#define FAILURE_GENERIC -1 // Falló
#define ERROR_MEMORY -1    // Error de alojamiento de memoria
#define ERROR_FATAL 1      // Error irrecuperable
#define ERROR_ARG_NOVAL 2  // Error en argumentos

// Apertura de arhivos
#define ERROR_APERTURA_ARCHIVO 3
#define ERROR_CIERRE_ARCHIVO 4

// Error de pipes
#define ERROR_PIPE_SER_CTE 5
#define ERROR_PIPE_CTE_SER 6
#define ERROR_COMUNICACION 7

// Lectura / Escritura
#define ERROR_LECTURA 8
#define ERROR_ESCRITURA 9

// Otros errores
#define ERROR_PID_NOT_EXIST -1

#endif // __COMMON_H__