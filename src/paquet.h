/**
 * @file paquet.h
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
#include "book.h"     // Para usar estructuras de libros

/**
 * @enum PAQUET_TYPE_T
 * @brief Tipos de datos que pueden ser enviados con el paquete
 */
enum PAQUET_TYPE_T
{
    SIGNAL, /**< asociado struct \ref PAQUET_SIGNAL_T*/
    BOOK,   /**< asociado struct \ref book*/
    ERR     /**< NO TIENE TIPO DE DATO ASOCIADO (Sólo señalar errores)*/
};

/**
 * @struct PAQUET_SIGNAL_T
 * @brief Estructura que compone una señal
 */
struct PAQUET_SIGNAL_T
{
    int code;                /**< Código de la señal*/
    char buffer[TAM_STRING]; /**< Buffer opcional*/
};

/**
 * @union PAQUET_DATATYPE_T
 * @brief Datos del mensaje transmitido por \ref paquet_t,
 * puede ser de tipo señal o de tipo libro
 */
union PAQUET_DATATYPE_T
{
    struct PAQUET_SIGNAL_T signal; /**< Datos de la señal*/
    book_t libro;           /**< Datos del libro*/
};

/* ------------------------- ! ESTRUCTURA A USAR ¡ ------------------------- */

/*
Esta es la estructura que se mandará a través de los pipes, contiene, 
el pid del proceso que lo envía/reciba (cuando aplique*), el tipo de
datos que contiene (datos o señal), y contiene los datos transmitidos
*/

/**
 * @struct paquet_t
 * @brief Dato que será enviado a través de los pipes, también conocido
 * como Paquete o Mensaje
 */
typedef struct
{
    pid_t client;                 /**< PID del cliente que envió o recibe el paquete*/
    enum PAQUET_TYPE_T type;      /**< Tipo del paquete*/
    union PAQUET_DATATYPE_T data; /**< Contenido del paquete*/
} paquet_t;

#endif // __DATA_H__