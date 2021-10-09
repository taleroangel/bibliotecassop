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

#ifndef __BOOK_H__
#define __BOOK_H__

// Tipo de petición al Servidor
enum PETICION
{
    SOLICITAR,
    RENOVAR,
    DEVOLVER
};

// Datatype para los libros
struct BOOK_T
{
    enum PETICION petition; // Tipo de petición que se solicita
};

#endif // __BOOK_H__