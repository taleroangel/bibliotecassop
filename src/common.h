/**
 * @file common.h
 * @authors  Ángel David Talero
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

#define TAM_STRING 100  /**< Tamaño de los Strings*/
#define WEEK_SEC 604800 /**< Una semana en segundos*/

#define PERMISOS_PIPE S_IRWXU   /**< Permiso para Leer, Escribir, Ejecutar*/
#define INTENTOS_ESCRITURA 5    /**< Intentos de escritura en caso de falla*/
#define TIMEOUT_COMUNICACION 10 /**< Tiempo límite (s) para establecer comunicación*/

#define WRITE 0 /**< Definiciones para el vector fd*/
#define READ 1  /**< Definiciones para el vector fd*/

/* -------------------------------- Señales -------------------------------- */
/* ------------------------- Señales de peticiones ------------------------- */
#define PET_ERROR -3 /**< Error de petición*/
#define SOLICITUD 3  /**< Solicitud exitosa*/
#define RENOVACION 4 /**< Renovación exitosa*/
#define DEVOLUCION 5 /**< Devolución exitosa*/

/* ---------------- Señales de confirmación de comunicación ---------------- */

#define START_COM 1   /**< Señal para empezar comunicación*/
#define STOP_COM -1   /**< Señal para detener confirmación*/
#define SUCCEED_COM 2 /**< Señal de confirmación de comunicación*/
#define FAILED_COM -2 /**< Señal de fallo en la comunicación (TERMINACION)*/

/* --------------------------- Lista de errores --------------------------- */
/**< Errores genéricos*/
#define SUCCESS_GENERIC 0  /**< Exitoso*/
#define FAILURE_GENERIC -1 /**< Falló*/
#define ERROR_FATAL 1      /**< Error irrecuperable*/

/**< Apertura de arhivos*/
#define ERROR_APERTURA_ARCHIVO 2 /**< Error en la apertura de un archivo*/
#define ERROR_CIERRE_ARCHIVO 3   /**< Error en el cierre de un archivo*/

/**< Error de pipes*/
#define ERROR_PIPE_SER_CTE 4 /**< Error en el pipe (Servidor->Cliente)*/
#define ERROR_PIPE_CTE_SER 5 /**< Error en el pipe (Cliente->Servidor)*/
#define ERROR_COMUNICACION 6 /**< Error de comunicación*/

/**< Lectura / Escritura*/
#define ERROR_LECTURA 7   /**< Error de lectura de un archivo*/
#define ERROR_ESCRITURA 8 /**< Error de escritura de un archivo*/

/**< Otros errores*/
#define ERROR_ARG_NOVAL 9      /**< Error en argumentos*/
#define ERROR_MEMORY 10        /**< Error de alojamiento de memoria*/
#define ERROR_PID_NOT_EXIST 11 /**< PID del cliente no existe*/
#define ERROR_SOLICITUD 12     /**< Solicitud inválida*/

#endif /**< __COMMON_H__*/