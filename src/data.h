/**
 * @file data.h
 * @author  Ángel David Talero
 *          Juan Esteban Urquijo
 *          Humberto Rueda Cataño
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

// Tipo de informacion que se transmite (SEÑAL / LIBRO)
enum TYPE_T
{
    SIGNAL,
    BOOK
};

// Estructura de un libro
struct BOOK_T
{
};

// Estructura de una señal
struct SIGNAL_T
{
    int code;                // Código de la señal
    char buffer[TAM_STRING]; // Buffer opcional
};

// Datos a transmitir ya sea un libro o una señal
union IN_DATA_T
{
    struct SIGNAL_T signal;
    struct BOOK_T book;
};

/* ------------------------- ! ESTRUCTURA A USAR ¡ ------------------------- */

/*
Esta es la estructura que se mandará a través de los pipes, contiene, 
el pid del proceso que lo envía/reciba (cuando aplique*), el tipo de
datos que contiene (datos o señal), y contiene los datos transmitidos
*/

typedef struct
{
    pid_t client;
    enum TYPE_T type;
    union IN_DATA_T data;
} data_t;

#endif // __DATA_H__