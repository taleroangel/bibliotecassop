/**
 * @file buffer.h
 * @authors Ángel David Talero
 *          Juan Esteban Urquijo
 *          Humberto Rueda Cataño
 * @brief Proceso solicitante
 * @copyright 2021
 * Pontificia Universidad Javeriana
 * Facultad de Ingeniería
 * Bogotá D.C - Colombia
 */

#ifndef __BUFFER_H__
#define __BUFFER_H__

#include "paquet.h"

/**
 * @struct buffer_t
 * @brief Arreglo dinámico de peticiones
 * 
 */
typedef struct
{
    int n_items;           /**< Número de paquetes en el arreglo*/
    paquet_t *paquetArray; /**< Arreglo de paquetes en cola*/
} buffer_t;

/**
 * @brief 
 * 
 * @param buffer_peticiones Cola con las peticiones
 * @param paquete Paquete a insertar
 * @return SUCCESS_GENERIC si éxito, cualquier otro valor de lo contrario
 */
int queue(buffer_t *buffer_peticiones, paquet_t paquete);

/**
 * @brief Obtener el próximo paquete en la Cola
 * @note No olvidar retirarlo con \ref dequeue
 * 
 * @param buffer_peticiones Cola con las peticiones
 * @return El paquete en la cola o NULL si no hay (Apuntador)
 */
paquet_t *getLast(buffer_t *buffer_peticiones);

/**
 * @brief Eliminar el último paquete de la cola
 * 
 * @param buffer_peticiones Cola con las peticiones
 * @return SUCCESS_GENERIC si éxito, FAILURE_GENERIC si está vacío
 */
int dequeue(buffer_t *buffer_peticiones);

#endif // __BUFFER_H__