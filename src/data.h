/**
 * @file data.h
 * @authors  Ángel David Talero,
 *          Juan Esteban Urquijo,
 *          Humberto Rueda Cataño,
 * @brief Estructuras que se movilizan a través del pipe
 * @copyright 2021
 * Pontificia Universidad Javeriana
 * Facultad de Ingeniería
 * Bogotá D.C - Colombia
 */

#ifndef __DATA_H__
#define __DATA_H__

#include <sys/types.h> // Para usar pid_t
#include "common.h"    // Para usar byte
#include "libro.h"     // Para usar estructuras de libros

/**
 * @enum TYPE_T
 * @brief Tipos de datos que pueden ser enviados con el paquete
 */
enum TYPE_T
{
    SIGNAL, /**< asociado struct \ref SIGNAL_T*/
    BOOK,   /**< asociado struct \ref ejemplar*/
    ERR     /**< NO TIENE TIPO DE DATO ASOCIADO (Sólo señalar errores)*/
};

/**
 * @struct SIGNAL_T
 * @brief Estructura de la SEÑAL
 */
struct SIGNAL_T
{
    int code;                /**< Código de la señal*/
    char buffer[TAM_STRING]; /**< Buffer opcional*/
};

/**
 * @union IN_DATA_T
 * @brief Datos a transmitir ya sea un libro o una señal
 */
union IN_DATA_T
{
    struct SIGNAL_T signal; /**< Datos de la señal*/
    struct ejemplar libro;  /**< Datos del libro*/
};

/* ------------------------- ! ESTRUCTURA A USAR ¡ ------------------------- */

/*
Esta es la estructura que se mandará a través de los pipes, contiene, 
el pid del proceso que lo envía/reciba (cuando aplique*), el tipo de
datos que contiene (datos o señal), y contiene los datos transmitidos
*/

/**
 * @struct data_t
 * @brief Dato que será enviado a través de los pipes
 */
typedef struct
{
    pid_t client;         /**< PID del cliente que envió o recibe el paquete*/
    enum TYPE_T type;     /**< Tipo del paquete*/
    union IN_DATA_T data; /**< Contenido del paquete*/
} data_t;

#endif // __DATA_H__