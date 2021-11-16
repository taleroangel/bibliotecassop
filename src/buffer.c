#include <stdlib.h>
#include "buffer.h"
#include "common.h"

int queue(buffer_t *buffer_peticiones, paquet_t paquete)
{
    // Add 1 to the paquet counter
    buffer_peticiones->n_items++;

    // Realloc memory for the new paquet
    paquet_t *aux = // Hacer el vector más grande y guardarlo en un nuevo apuntador
        (paquet_t *)realloc(
            buffer_peticiones->paquetArray,
            sizeof(paquet_t) * buffer_peticiones->n_items);

    // If allocation failed
    if (aux == NULL)
    {
        perror("Error");
        fprintf(stderr, "No es posible agregar un nuevo paquete...");
        return ERROR_MEMORY;
    }

    // Set the new pointer
    buffer_peticiones->paquetArray = aux;

    // Move everything to the left
    for (int i = buffer_peticiones->n_items - 1; i > 0; i--)
    {
        buffer_peticiones->paquetArray[i] = buffer_peticiones->paquetArray[i - 1];
        // No se pierde ninguna posición, pero se repite la posición 0
    }

    // Store the new paquet
    buffer_peticiones->paquetArray[0] = paquete;

    return SUCCESS_GENERIC;
}

paquet_t *getLast(buffer_t *buffer_peticiones)
{
    if (buffer_peticiones->n_items == 0)
        return NULL;
    return &buffer_peticiones->paquetArray[buffer_peticiones->n_items - 1];
}

int dequeue(buffer_t *buffer_peticiones)
{
    // Realloc the array
    if (buffer_peticiones->n_items >= 1)
        buffer_peticiones->n_items--;
    else
        return FAILURE_GENERIC;

    // Realloc memory for the new paquet
    paquet_t *aux = // Hacer el vector más grande y guardarlo en un nuevo apuntador
        (paquet_t *)realloc(
            buffer_peticiones->paquetArray,
            sizeof(paquet_t) * buffer_peticiones->n_items);

    // If allocation failed
    if (aux == NULL)
    {
        perror("Error");
        fprintf(stderr, "No es posible eliminar un paquete...\n");
        return ERROR_MEMORY;
    }

    // Set the new pointer
    buffer_peticiones->paquetArray = aux;

    return SUCCESS_GENERIC;
}
