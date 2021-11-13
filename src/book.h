/**
 * @file book.h
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

#define MAX_CANT_LIBROS 100 /**< Máxima cantidad de libros en el arreglo*/

//-----Estructuras de datos-----//

/**
 * @enum BOOK_REQUEST_T
 * @brief Describe los diferentes tipos de peticiones para libros que puede enviar
 * el Cliente al Servidor 
 */
enum BOOK_REQUEST_T
{
    SOLICITAR,
    RENOVAR,
    DEVOLVER,
    BUSCAR
};

/**
 * @struct copy_t
 * @brief Describe la información adicional de cada uno de los
 * ejemplares que componen un libro
 */
typedef struct
{
    int n_copy;            /**< Número de ejemplar*/
    char state;            /**< Estado (D o P)*/
    char date[TAM_STRING]; /**< Fecha de préstamo (dd-mm-YY)*/

} copy_t;

/**
 * @struct book_t
 * @brief Información de cada uno de los libros
 * @note Para almacenamiento de los ejemplares como para realizar peticiones 
 * se usa la misma estructura, es por esto que algunos campos son opcionales
 */
typedef struct
{
    enum BOOK_REQUEST_T petition; /**< Tipo de petición que se solicita*/

    int ISBN;              /**< ISBN del libro*/
    char name[TAM_STRING]; /**< Nombre del libro*/

    int n_copies;    /**< Cantidad de ejemplares*/
    copy_t copyInfo; /**< Información adicional de ejemplares*/

} book_t;

//? Aquí había una variable declarada global dentro del header,
//? moví la declaración al main porque fue una de las recomendaciones

#endif