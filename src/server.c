/**
 * @file server.c
 * @author  Ángel David Talero
 *          Juan Esteban Urquijo
 *          Humberto Rueda Cataño
 * @brief Proceso receptor de peticiones
 * @copyright 2021
 * Pontificia Universidad Javeriana
 * Facultad de Ingeniería
 * Bogotá D.C - Colombia
 */

/* -------------------------------  Libraries ------------------------------- */
// ISO C libraries
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// POSIX syscalls
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

// Header propias
#include "server.h"
#include "common.h"
#include "data.h"

/* --------------------------------- Main --------------------------------- */
int main(int argc, char *argv[])
{
    char nombrePipe[TAM_STRING],
        archivoEntrada[TAM_STRING],
        archivoSalida[TAM_STRING];

    // Manejar los argumentos
    manejarArgumentos(argc, argv, nombrePipe, archivoEntrada, archivoSalida);

    // Iniciar la comunicación
    int inputPipe = iniciarComunicacion(nombrePipe);

    // Lista de los clientes;
    struct client_list clients;

    clients.nClients = 0;
    clients.clientArray = (client_t *)malloc(sizeof(client_t));

    // Agregar clientes
    int rr = 0;
    do
    {
        data_t readData;
        //rr = read(inputPipe, &readData, sizeof(readData));
        sleep(10);
        rr++;

    } while (rr != 0);

    // Eliminar lista de clientes
    free(clients.clientArray);
    unlink(nombrePipe);
}

/* ----------------------------- Definiciones ----------------------------- */

/* ------------------------- Manejo de argumentos ------------------------- */

void mostrarUso(void)
{
    fprintf(stderr,
            "Uso: ./server -p pipeReceptor -f baseDeDatos -s archivoSalida\n");
    exit(ERROR_ARG_NOVAL);
}

void manejarArgumentos(int argc,
                       char *argv[],
                       char *pipeNom,
                       char *fileIn,
                       char *fileOut)
{
    // Todos los argumentos son obligatorios
    if (argc != 7)
        mostrarUso();

    // Filtrar los argumentos
    bool argPipe = false, argIn = false, argOut = false;

    while ((argc > 1) && (argv[1][0] == '-'))
    {
        switch (argv[1][1])
        {
        case 'p':
            // Verificar si ya se usó el argPipeumento
            if (argPipe)
            {
                printf("El argumento %s ya fue utilizado!\n", argv[1]);
                mostrarUso();
            }

            // El argPipeumento ya fue utilizado!
            argPipe = true;

            // retornar el nombre de archivo
            strcpy(pipeNom, argv[2]);

            break;

        case 'f':
            // Verificar si ya se usó el argInumento
            if (argIn)
            {
                printf("El argumento %s ya fue utilizado!\n", argv[1]);
                mostrarUso();
            }

            // El argInumento ya fue utilizado!
            argIn = true;

            // retornar el nombre de archivo
            strcpy(fileIn, argv[2]);

            break;

        case 's':
            // Verificar si ya se usó el argOutumento
            if (argOut)
            {
                printf("El argumento %s ya fue utilizado!\n", argv[1]);
                mostrarUso();
            }

            // El argOutumento ya fue utilizado!
            argOut = true;

            // retornar el nombre de archivo
            strcpy(fileOut, argv[2]);

            break;

        default:
            printf("Argumento no válido: %s\n", argv[1]);
            mostrarUso();
        }

        argv += 2; // Mover el puntero de argumentos
        argc -= 2; // Reducir cantidad de argumentos para el while
    }
}

/* ----------------------- Protocolos de comunicación ----------------------- */

int iniciarComunicacion(const char *pipeCTE_SER)
{
    // Crear el pipe (Cliente -> Servidor)
    unlink(pipeCTE_SER);
    if (mkfifo(pipeCTE_SER, PERMISOS_PIPE) < 0)
    {
        perror("Error de comunicación"); // Manejar Error
        exit(ERROR_PIPE_SER_CTE);
    }

    // Abrir el pipe para lectura
    int pipe = open(pipeCTE_SER, O_RDONLY);
    if (pipe < 0)
    {
        perror("Error de comunicación"); // Manejar Error
        exit(ERROR_PIPE_CTE_SER);
    }

    return pipe;
}

int conectarCliente(struct client_list *clients, data_t package)
{
    if (package.type != SIGNAL && package.data.signal.code != START_COM)
    {
        fprintf(stderr, "Unexpected instruction\n");
        return ERROR_COMUNICACION;
    }

    // Try to open the pipe
    int pipefd = open(package.data.signal.buffer, O_WRONLY);
    if (pipefd < 0)
    {
        perror("Error en comunicación");
        return ERROR_PIPE_SER_CTE;
    }

    // Save the client
    clients->nClients++;

    client_t *aux =
        (client_t *)realloc(
            clients->clientArray,
            sizeof(client_t) * clients->nClients);

    // If allocation failed
    if (aux == NULL)
    {
        perror("Error");
        fprintf(stderr, "No es posible agregar un nuevo cliente...");
        return ERROR_MEMORY;
    }

    // Set the new pointer
    clients->clientArray = aux;

    // Ahora agregar el nuevo cliente
    int newClient = clients->nClients - 1;

    clients->clientArray[newClient].clientPID = package.client;
    strcpy(clients->clientArray[newClient].pipeNom, package.data.signal.buffer);
    clients->clientArray[newClient].pipe = pipefd;

    return SUCCESS;
}