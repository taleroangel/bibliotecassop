/**
 * @file libro.h
 * @authors  Ángel David Talero
 *          Juan Esteban Urquijo
 *          Humberto Rueda Cataño
 * @brief Estrucuras que representan los libros
 * @copyright 2021
 * Pontificia Universidad Javeriana
 * Facultad de Ingeniería
 * Bogotá D.C - Colombia
 */

#ifndef _LIBRO_H_
#define _LIBRO_H_

//-----Librerias-----//
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#include "common.h"

//-----Variables globales-----//

#define MAX_CANT_LIBROS 100
#define MAX_CANT_LINEAS 1000

//-----Estructuras de datos-----//

/**
 * @enum PETICION
 * @brief Describe los diferentes tipos de peticiones para libros que puede enviar
 * el Cliente al Servidor 
 */
enum PETICION
{
    SOLICITAR,
    RENOVAR,
    DEVOLVER,
    BUSCAR
};

/**
 * @struct libro
 * @brief Describe la información adicional de cada uno de los ejemplares que componen
 * un libro
 */
struct libro
{
    int numero;             /**< Número de ejemplar*/
    char estado;            /**< Estado (D o P)*/
    char fecha[TAM_STRING]; /**< Fecha de préstamo (dd-mm-YY)*/
};

/**
 * @struct ejemplar
 * @brief Información de cada uno de los libros
 */
struct ejemplar
{
    enum PETICION petition; /**< Tipo de petición que se solicita*/

    int isbn;                /**< ISBN del libro*/
    int num_ejemplar;        /**< Cantidad de ejemplares*/
    char nombre[TAM_STRING]; /**< Nombre del libro*/
    struct libro libroEjem;  /**< Información adicional de ejemplares*/

} ejemplar[MAX_CANT_LIBROS];

#endif