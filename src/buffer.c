#include <stdlib.h>
#include "buffer.h"
#include "common.h"

#include <semaphore.h>

sem_t available_resources = {0};
sem_t available_spaces = {0};

int init(buffer_t *buffer_peticiones)
{
    if (buffer_peticiones == NULL)
        return FAILURE_GENERIC;

    buffer_peticiones->current_item = 0;

    // Initialize semaphores with resources
    if (sem_init(&available_resources, 0, 0) ||
        sem_init(&available_spaces, 0, BUFFER_SIZE))
    {
        perror("Hilo auxiliar");
        return FAILURE_GENERIC;
    }

    return SUCCESS_GENERIC;
}

int queue(buffer_t *buffer_peticiones, paquet_t paquete)
{
    if (buffer_peticiones == NULL)
        return FAILURE_GENERIC;

    // Esperar que haya un espacio disponible en cola
    if (sem_wait(&available_spaces))
    {
        perror("Hilo auxiliar");
        return FAILURE_GENERIC;
    }

    // Agregar a la cola
    buffer_peticiones->paquetArray[buffer_peticiones->current_item] = paquete;

    // Otorgar un recurso
    if (sem_post(&available_resources))
    {
        perror("Hilo auxiliar");
        return FAILURE_GENERIC;
    }

    return SUCCESS_GENERIC;
}

int destroy(buffer_t *buffer_peticiones)
{
    if (buffer_peticiones == NULL)
        return FAILURE_GENERIC;

    if (sem_destroy(&available_spaces) || sem_destroy(&available_resources))
    {
        perror("Hilo auxiliar");
        return FAILURE_GENERIC;
    }
    return SUCCESS_GENERIC;
}

paquet_t *getNext(buffer_t *buffer_peticiones)
{
    if (buffer_peticiones == NULL)
        return NULL;

    // Esperar a que haya un recurso disponible en cola
    sem_wait(&available_resources);
    return &buffer_peticiones->paquetArray[buffer_peticiones->current_item];
}

int dequeue(buffer_t *buffer_peticiones)
{
    if (buffer_peticiones == NULL)
        return FAILURE_GENERIC;

    if (sem_post(&available_spaces))
    {
        perror("Hilo auxiliar");
        return FAILURE_GENERIC;
    }

    // Mover al siguiente elemento
    buffer_peticiones->current_item = (buffer_peticiones->current_item + 1) % BUFFER_SIZE;
    return SUCCESS_GENERIC;
}