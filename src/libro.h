/**
 * @file book.h
 * @author  Ángel David Talero
 *          Juan Esteban Urquijo
 *          Humberto Rueda Cataño
 * @brief Estrucura de libros
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

#define MAX_NOMLIBRO 100
#define MAX_CANT_LIBROS 100
#define MAX_CANT_LINEAS 1000
#define NOMBRE_ARCHIVO "BD.txt"

//-----Estructuras de datos-----//

// Tipo de petición al Servidor
enum PETICION
{
    SOLICITAR,
    RENOVAR,
    DEVOLVER,
    BUSCAR
};

struct libro
{
    int numero;
    char estado;
    char fecha[TAM_STRING];
};

struct ejemplar
{
    enum PETICION petition; // Tipo de petición que se solicita

    int isbn;
    int num_ejemplar;
    char nombre[MAX_NOMLIBRO];
    struct libro libroEjem;

} ejemplar[MAX_CANT_LIBROS];

#endif