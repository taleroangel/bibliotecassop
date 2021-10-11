/**
 * @file book.h
 * @author  Ángel David Talero
 *          Juan Esteban Urquijo
 *          Humberto Rueda Cataño
 * @brief Estrucura de peticiones
 * @copyright 2021
 * Pontificia Universidad Javeriana
 * Facultad de Ingeniería
 * Bogotá D.C - Colombia
 */

#ifndef __PETICION_H__
#define __PETICION_H__

#include "libro.h"

/**
 * @struct peticion_t
 * Estructura en la cual se almacenan las diferentes peticiones que se leen
 * por archivo en el proceso solicitante (Cliente)
 */
struct peticion_t
{
    char peticion;
    char nombre[TAM_STRING];
    int isbn;
};

#endif // __PETICION_H__