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

#include <stdbool.h>
#include "paquet.h"

#define BUFFER_SIZE 10 /**< Tamaño estático del buffer*/

/**
 * @struct buffer_t
 * @brief Arreglo dinámico de peticiones
 * 
 */
typedef struct
{
    int current_item;                  /**< Paquete actual del arreglo*/
    paquet_t paquetArray[BUFFER_SIZE]; /**< Arreglo de paquetes en cola*/
} buffer_t;

/**
 * @brief Iniciar el buffer
 * 
 * @param buffer_peticiones Apuntador al buffer
 * @return int SUCCESS_GENERIC o FAILURE_GENERIC
 */
int init(buffer_t *buffer_peticiones);

/**
 * @brief Cerrar el buffer
 * 
 * @param buffer_peticiones Apuntador al buffer
 * @return int SUCCESS_GENERIC o FAILURE_GENERIC
 */
int destroy(buffer_t *buffer_peticiones);

/**
 * @brief 
 * 
 * @param buffer_peticiones Cola con las peticiones
 * @param paquete Paquete a insertar
 * @return SUCCESS_GENERIC si éxito, FAILURE_GENERIC si no hay espacio
 */
int queue(buffer_t *buffer_peticiones, paquet_t paquete);

/**
 * @brief Obtener el próximo paquete en la Cola
 * @note No olvidar retirarlo con \ref dequeue
 * 
 * @param buffer_peticiones Cola con las peticiones
 * @return El paquete en la cola o NULL si no hay (Apuntador)
 */
paquet_t *getNext(buffer_t *buffer_peticiones);

/**
 * @brief Eliminar el último paquete de la cola
 * 
 * @param buffer_peticiones Cola con las peticiones
 * @return SUCCESS_GENERIC si éxito, FAILURE_GENERIC si está vacío
 */
int dequeue(buffer_t *buffer_peticiones);

#endif // __BUFFER_H__