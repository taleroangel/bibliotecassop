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
    char pipeCTE_SER[TAM_STRING],
        archivoEntrada[TAM_STRING],
        archivoSalida[TAM_STRING];

    // Manejar los argumentos
    manejarArgumentos(argc, argv, pipeCTE_SER, archivoEntrada, archivoSalida);

    // Iniciar la comunicación
    int pipeRead = startCommunication(pipeCTE_SER);

    // Lista de los clientes;
    struct client_list clients;
    clients.nClients = 0;
    clients.clientArray = (client_t *)malloc(sizeof(client_t));

    // Paquete temporal donde se guarda lo recibido por el pipe
    data_t package;
    int return_status;

    // Leer contenidos del pipe continuamente hasta que no hayan lectores
    while (read(pipeRead, &package, sizeof(package)) != 0)
    {
        // Interpretar el tipo de mensaje
        switch (package.type)
        {
        case SIGNAL:
            return_status = interpretSignal(&clients, package);
            if (return_status != SUCCESS_GENERIC)
            {
                fprintf(stderr,
                        "Hubo un problema en la lectura de un paquete\n");
                fprintf(stderr, "SEÑAL: Código de error: %d\n", return_status);
                perror("SO");
            }
            break;

        case BOOK:
            return_status = -1; // Aquí va la función que maneja los libros
            if (return_status != SUCCESS_GENERIC)
            {
                fprintf(stderr,
                        "Hubo un problema en la lectura de un paquete\n");
                fprintf(stderr, "LIBRO: Código de error: %d\n", return_status);
                perror("SO");
            }
            break;

        default:
            fprintf(stderr, "Hubo un problema en la lectura de un paquete\n");
            break;
        }
    }

    // Notificación
    fprintf(stdout,
            "Todos los clientes se han desconectado, cerrando el servidor...\n");

    // Eliminar lista de clientes
    free(clients.clientArray);

    // Deshacer el pipe de Servidor
    close(pipeRead);
    unlink(pipeCTE_SER);

    // Notificación
    fprintf(stdout,
            "Se ha cerrado el pipe (Cliente->Servidor)...\n");

    return EXIT_SUCCESS;
}

/* ----------------------------- Definiciones ----------------------------- */

/* ------------------------- Manejo de argumentos ------------------------- */

void mostrarUso(void)
{
    fprintf(stdout,
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
                fprintf(stdout, "El argumento %s ya fue utilizado!\n", argv[1]);
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
                fprintf(stdout, "El argumento %s ya fue utilizado!\n", argv[1]);
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
                fprintf(stdout, "El argumento %s ya fue utilizado!\n", argv[1]);
                mostrarUso();
            }

            // El argOutumento ya fue utilizado!
            argOut = true;

            // retornar el nombre de archivo
            strcpy(fileOut, argv[2]);

            break;

        default:
            fprintf(stdout, "Argumento no válido: %s\n", argv[1]);
            mostrarUso();
        }

        argv += 2; // Mover el puntero de argumentos
        argc -= 2; // Reducir cantidad de argumentos para el while
    }
}

/* ----------------------- Protocolos de comunicación ----------------------- */

int startCommunication(const char *pipeCTE_SER)
{
    // Crear el pipe (Cliente -> Servidor)

    unlink(pipeCTE_SER);
    if (mkfifo(pipeCTE_SER, PERMISOS_PIPE) < 0)
    {
        perror("Error de comunicación"); // Manejar Error
        exit(ERROR_PIPE_SER_CTE);
    }

    // Notificación
    fprintf(stdout, "Notificación: Se ha creado el pipe (Cliente->Servidor)\n");
    fprintf(stdout, "Notificación: El servidor está en estado de espera...\n");

    // Abrir el pipe para lectura (Cliente->Servidor)
    int pipe = open(pipeCTE_SER, O_RDONLY);
    if (pipe < 0)
    {
        perror("Error de comunicación"); // Manejar Error
        exit(ERROR_PIPE_CTE_SER);
    }

    // Notificación
    fprintf(stdout, "Notificación: Se ha abierto el pipe (Cliente->Servidor)\n");

    return pipe;
}

int connectClient(struct client_list *clients, data_t package)
{
    // Notificación
    fprintf(stdout, "\nUn nuevo cliente está iniciando una conexión\n");

    if (package.type != SIGNAL && package.data.signal.code != START_COM)
    {
        fprintf(stderr, "Unexpected instruction\n");
        return ERROR_COMUNICACION;
    }

    //!5. Servidor abre el pipe (Servidor->Cliente) para ESCRITURA
    //Try to open the pipe
    int pipefd = open(package.data.signal.buffer, O_WRONLY);
    if (pipefd < 0)
    {
        perror("Error en comunicación");
        return ERROR_PIPE_SER_CTE;
    }

    // Notificación
    fprintf(stdout,
            "Notificación: El pipe (Servidor->Cliente) fue abierto!: %s\n",
            package.data.signal.buffer);

    //!6. Servidor guarda la información de Cliente con su respectivo pipe
    //!de comunicación

    // Leer los datos en el paquete y convertirlo en cliente
    client_t nuevo = createClient(
        pipefd, package.client, package.data.signal.buffer);

    // Guardar el nuevo cliente
    if (storeClient(clients, nuevo) != SUCCESS_GENERIC)
        return ERROR_MEMORY;

    // Notificación
    fprintf(stdout,
            "Notificación: Nuevo cliente agregado\n");

    //!7. Servidor envía una señal de confirmación a Cliente

    // Crear una señal
    data_t toSent;
    toSent.type = SIGNAL;
    toSent.client = nuevo.clientPID;
    toSent.data.signal.code = SUCCEED_COM;

    // En caso de que el pipe se cierre justo en el envío de la señal
    if (write(nuevo.pipe, &toSent, sizeof(toSent)) < 0)
    {
        perror("Error");
        fprintf(stderr, "Pipe cerrado inesperadamente");

        // Desalojar recursos
        removeClient(clients, nuevo.clientPID);
        close(pipefd);

        return ERROR_PIPE_SER_CTE;
    }

    // Notificación
    fprintf(stdout,
            "Notificación: Señal enviada\n");

    fprintf(stdout,
            "La comunicación fue exitosa\n");

    return SUCCESS_GENERIC;
}

int disconnectClient(struct client_list *clients, data_t package)
{
    // Notificación
    fprintf(stdout, "\nUn cliente abandona la comunicación\n");

    if (package.type != SIGNAL && package.data.signal.code != STOP_COM)
    {
        fprintf(stderr, "Unexpected instruction\n");
        return ERROR_COMUNICACION;
    }

    pid_t client = package.client;
    int pipeSER_CTE = searchClient(clients, client);

    //! 2. Servidor cierra la escritura del pipe (Servidor->Cliente)
    if (close(pipeSER_CTE) < 0)
    {
        perror("Error");
        return ERROR_PIPE_SER_CTE;
    }

    //!3. Servidor actualiza la lista de clientes
    int temp = removeClient(clients, client);
    if (temp != SUCCESS_GENERIC)
    {
        fprintf(stderr, "No se pudo borrar el cliente");
        return temp;
    }

    // Notificación
    fprintf(stdout,
            "Comunicación cerrada exitosamente\n");
    return SUCCESS_GENERIC;
}

int interpretSignal(struct client_list *clients, data_t package)
{
    switch (package.data.signal.code)
    {
    case START_COM:
        return connectClient(clients, package);
        break;

    case STOP_COM:
        return disconnectClient(clients, package);
        break;
    }

    return FAILURE_GENERIC;
}

data_t generateReponse(pid_t dest, int code, char *buffer)
{
    // Paquet creation
    data_t reponse;
    reponse.client = dest;
    reponse.type = SIGNAL;

    // Signal construction
    reponse.data.signal.code = code;
    if (buffer != NULL)
        strcpy(reponse.data.signal.buffer, buffer);

    return reponse;
}

/* --------------------------- Manejo de clientes --------------------------- */

client_t createClient(int pipefd, pid_t clientpid, char *pipenom)
{
    client_t clienteNuevo;
    clienteNuevo.clientPID = clientpid;
    clienteNuevo.pipe = pipefd;
    strcpy(clienteNuevo.pipeNom, pipenom);
    return clienteNuevo;
}

int storeClient(struct client_list *clients, client_t client)
{
    // Add 1 to the client counter
    int pos_newClient = clients->nClients++;

    // Realloc memory for the new client
    client_t *aux = // Hacer el vector más grande y guardarlo en un nuevo apuntador
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

    // Store the new client
    clients->clientArray[pos_newClient] = client;

    return SUCCESS_GENERIC;
}

int removeClient(struct client_list *clients, pid_t clientToRemove)
{
    // Search for the client to remove and move it to the last position
    if (clients->clientArray[clients->nClients - 1].clientPID != clientToRemove)
    {
        bool found = false;
        for (int i = 0; i < clients->nClients; i++)
        {
            if (clients->clientArray[i].clientPID == clientToRemove)
            {
                found = true;

                // Set variables
                client_t toRemove = clients->clientArray[i];
                client_t last = clients->clientArray[clients->nClients - 1];

                // Swap
                clients->clientArray[i] = last;
                clients->clientArray[clients->nClients - 1] = toRemove;

                break;
            }
        }

        if (!found) // If PID was not found
            return ERROR_PID_NOT_EXIST;
    }

    // Realloc the array
    if (clients->nClients > 1)
        clients->nClients--;

    // Realloc memory for the new client
    client_t *aux = // Hacer el vector más grande y guardarlo en un nuevo apuntador
        (client_t *)realloc(
            clients->clientArray,
            sizeof(client_t) * clients->nClients);

    // If allocation failed
    if (aux == NULL)
    {
        perror("Error");
        fprintf(stderr, "No es posible eliminar un cliente...\n");
        return ERROR_MEMORY;
    }

    // Set the new pointer
    clients->clientArray = aux;

    return SUCCESS_GENERIC;
}

int searchClient(struct client_list *clients, pid_t client)
{
    for (int i = 0; i < clients->nClients; i++)
        if (clients->clientArray[i].clientPID == client)
            return clients->clientArray[i].pipe;

    return ERROR_PID_NOT_EXIST;
}
