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

/* ------------------------------  Libraries ------------------------------ */
#define _XOPEN_SOURCE // Para la función strptime()

// ISO C libraries
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

// POSIX syscalls
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

// Header propias
#include "server.h"
#include "common.h"
#include "data.h"
#include "libro.h"

/* --------------------------------- Main --------------------------------- */
int main(int argc, char *argv[])
{
    char pipeCTE_SER[TAM_STRING],
        archivoEntrada[TAM_STRING],
        archivoSalida[TAM_STRING];

    // Manejar los argumentos
    manejarArgumentos(argc, argv, pipeCTE_SER, archivoEntrada, archivoSalida);

    // Abrir la base de datos
    FILE *archivo = fopen(archivoEntrada, "r");
    if (archivo == NULL)
    {
        printf("No se puede abrir el archivo\n");
        return ERROR_APERTURA_ARCHIVO;
    }

    // Iniciar la comunicación
    int pipeRead = startCommunication(pipeCTE_SER);

    // Lista de los clientes;
    struct client_list clients;
    clients.nClients = 0;
    clients.clientArray = (client_t *)malloc(sizeof(client_t));

    // Leer cada libro
    int n_libro = 0;
    for (int i = 0; i < MAX_CANT_LIBROS; i++)
    {
        fscanf(archivo, "%[^,],%d,%d\n",
               ejemplar[n_libro].nombre,
               &ejemplar[n_libro].isbn,
               &ejemplar[n_libro].num_ejemplar);

        int aux = ejemplar[n_libro].num_ejemplar;

        for (int j = 0; j < aux; j++)
        {
            if (j > 0)
            {
                strcpy(ejemplar[n_libro].nombre, ejemplar[n_libro - 1].nombre);
                ejemplar[n_libro].isbn = ejemplar[n_libro - 1].isbn;
                ejemplar[n_libro].num_ejemplar = ejemplar[n_libro - 1].num_ejemplar;
            }

            fscanf(archivo, "%d,%c,%s\n",
                   &ejemplar[n_libro].libroEjem.numero,
                   &ejemplar[n_libro].libroEjem.estado,
                   ejemplar[n_libro].libroEjem.fecha);

            n_libro++;
        }
    }

    // Paquete temporal donde se guarda lo recibido por el pipe
    data_t package;
    int return_status;

    // Leer contenidos del pipe continuamente hasta que no hayan lectores
    while (read(pipeRead, &package, sizeof(package)) != 0)
    {
        // Interpretar el tipo de mensaje
        switch (package.type)
        {
        case SIGNAL: //! Cuando se recibe una SEÑAL
            return_status = interpretSignal(&clients, package);
            if (return_status != SUCCESS_GENERIC)
            {
                fprintf(stderr,
                        "Hubo un problema en la solicitud del cliente (%d)\n",
                        package.client);
                fprintf(stderr, "SEÑAL: Código de error: %d\n", return_status);
            }
            break;

        case BOOK:                                                      //! Cuando se recibe un LIBRO
            return_status = manejarLibros(&clients, package, ejemplar); // Aquí va la función que maneja los libros
            if (return_status != SUCCESS_GENERIC)
            {
                fprintf(stderr,
                        "Hubo un problema en la solicitud del cliente (%d)\n",
                        package.client);
                fprintf(stderr, "LIBRO: Código de error: %d\n", return_status);
            }
            break;

        default:
            fprintf(stderr, "Hubo un problema en la solicitud del cliente (%d)\n",
                    package.client);
            break;
        }
    }

    // Notificación
    fprintf(stdout,
            "\nTodos los clientes se han desconectado, cerrando el servidor...\n");

    // Cerrar el archivo
    fclose(archivo);

    // Eliminar lista de clientes
    free(clients.clientArray);

    // Deshacer el pipe de Servidor
    close(pipeRead);
    unlink(pipeCTE_SER);

    // Notificación
    fprintf(stdout,
            "Se ha cerrado el pipe (Cliente->Servidor)...\n");

    // Terminar el proceso
    printf("\nServidor finaliza correctamente\n");
    return EXIT_SUCCESS;
}

/* ----------------------------- Definiciones ----------------------------- */

/* ------------------------- Manejo de argumentos ------------------------- */

void mostrarUso(void)
{
    fprintf(stdout,
            //"Uso: ./server -p pipeReceptor -f baseDeDatos -s archivoSalida\n");
            "Uso: ./server -p pipeReceptor -f baseDeDatos\n");
    exit(ERROR_ARG_NOVAL);
}

void manejarArgumentos(int argc,
                       char *argv[],
                       char *pipeNom,
                       char *fileIn,
                       char *fileOut)
{
    // Todos los argumentos son obligatorios
    /*
    if (argc != 7)
        mostrarUso();
    */

    if (argc != 5)
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

            /*
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
        */

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
    fprintf(stdout, "\nEl cliente (%d) quiere abandonar la comunicación\n",
            package.client);

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

/* ---------------------------- Manejo de libros ---------------------------- */

int manejarLibros(
    struct client_list *clients,
    data_t package,
    struct ejemplar ejemplar[])
{
    // Notificación
    printf("\nSe recibió una solicitud del cliente (%d)\n", package.client);

    // Optener el pipe del cliente
    int pipeCliente = searchClient(clients, package.client);

    if (pipeCliente < 0)
    {
        perror("Error");
        return ERROR_COMUNICACION;
    }

    data_t respuesta;
    char buffer[TAM_STRING];

    switch (package.data.libro.petition)
    {
    case SOLICITAR: //! Petición de solicitud
    {
        // Notificar
        printf("La petición es de tipo: SOLICITAR\n");

        // 1. Buscar el libro
        struct ejemplar libro = package.data.libro;

        bool encontrado = false;
        for (int i = 0; i < MAX_CANT_LIBROS; i++)
        {
            if (ejemplar[i].isbn == libro.isbn &&
                (strcmp(ejemplar[i].nombre, libro.nombre) == 0))
            {
                encontrado = true;
            }
        }

        respuesta = generateReponse(package.client, ERROR, NULL);

        if (!encontrado)
        {
            fprintf(stderr, "El libro no fue encontrado...\n");
            if (write(pipeCliente, &respuesta, sizeof(respuesta)) < 0)
            {
                perror("Error");
                return ERROR_SOLICITUD;
            }
            return ERROR_SOLICITUD;
        }

        // Mostrar notificación
        printf("El libro '%s' fue encontrado\n", libro.nombre);

        // 2. Verificar si está disponible

        bool libroActualizado = false;
        for (int i = 0; i < MAX_CANT_LIBROS; i++)
        {
            if (ejemplar[i].isbn == libro.isbn &&
                (strcmp(ejemplar[i].nombre, libro.nombre) == 0) &&
                ejemplar[i].libroEjem.estado == 'D')
            {
                printf("El libro '%s' será actualizado\n", libro.nombre);

                // 3. Modificar el estado del libro

                // Actualizar libro
                // Actualizar su estado
                libroActualizado = true;
                ejemplar[i].libroEjem.estado = 'P';

                // Actualizar su fecha //! Tiene que ser dentro de 1 semana
                char fecha[TAM_STRING];
                memset(fecha, 0, sizeof(fecha));

                time_t t;
                struct tm *tm;

                t = time(NULL);
                tm = localtime(&t);

                //? Añadirle 1 semana
                tm->tm_sec += WEEK_SEC;
                mktime(tm);

                // Formatear la fecha
                strftime(fecha, TAM_STRING, "%d-%m-%Y", tm);

                printf("IMPORTANTE: El libro está prestado hasta: %s\n", fecha);

                strcpy(ejemplar[i].libroEjem.fecha, fecha);
                strcpy(buffer, fecha);

                // Añadir qué ejemplar fue el que se prestó
                //ADVERTENCIA: como ya no se necesita fecha la usamos de string auxiliar
                memset(fecha, 0, sizeof(fecha));
                sprintf(fecha, " (Ejemplar #%d)", ejemplar[i].libroEjem.numero);
                strcat(buffer, fecha);

                break;
            }
        }

        // 4. Avisar al cliente
        if (!libroActualizado)
        {
            respuesta = generateReponse(package.client, ERROR, NULL);
            fprintf(stderr, "El libro no está disponible\n");
            if (write(pipeCliente, &respuesta, sizeof(respuesta)) < 0)
            {
                perror("Error");
                return ERROR_SOLICITUD;
            }
            return ERROR_SOLICITUD;
        }
        else
        {
            // Enviar también la fecha
            respuesta = generateReponse(package.client, SOLICITUD, buffer);

            fprintf(stdout, "Solicitud exitosa (%d)\n", package.client);
            if (write(pipeCliente, &respuesta, sizeof(respuesta)) < 0)
            {
                perror("Error");
                return ERROR_SOLICITUD;
            }
            return SUCCESS_GENERIC;
        }
    }
    break;

    //TODO
    case RENOVAR: //! Petición de renovación
    {
        // Notificar
        printf("La petición es de tipo: RENOVAR\n");

        // 1. Buscar el libro
        struct ejemplar libro = package.data.libro;

        bool encontrado = false;
        for (int i = 0; i < MAX_CANT_LIBROS; i++)
        {
            if (ejemplar[i].isbn == libro.isbn &&
                (strcmp(ejemplar[i].nombre, libro.nombre) == 0))
            {
                encontrado = true;
            }
        }

        respuesta = generateReponse(package.client, ERROR, NULL);

        if (!encontrado)
        {
            fprintf(stderr, "El libro no fue encontrado...\n");
            if (write(pipeCliente, &respuesta, sizeof(respuesta)) < 0)
            {
                perror("Error");
                return ERROR_SOLICITUD;
            }
            return ERROR_SOLICITUD;
        }

        // Mostrar notificación
        printf("El libro '%s' fue encontrado\n", libro.nombre);

        // 2. Verificar si está //? OCUPADO
        bool libroActualizado = false;
        for (int i = 0; i < MAX_CANT_LIBROS; i++)
        {
            if (ejemplar[i].isbn == libro.isbn &&
                (strcmp(ejemplar[i].nombre, libro.nombre) == 0) &&
                ejemplar[i].libroEjem.estado == 'P' && //? P de PRESTADO
                ejemplar[i].libroEjem.numero == libro.libroEjem.numero)
            {
                printf("El libro '%s' será actualizado\n", libro.nombre);

                // 3. Modificar el estado del libro

                // Actualizar libro
                // Actualizar su estado
                libroActualizado = true;
                ejemplar[i].libroEjem.estado = 'P'; //? Se deja en PRESTADO

                // Actualizar su fecha
                //! A LA FECHA DE DEVOLUCIÓN QUE SE TENÍA se le suma 1 semana

                char fecha[TAM_STRING];
                memset(fecha, 0, sizeof(fecha));

                time_t t;
                struct tm *fechaLibro;

                t = time(NULL);
                fechaLibro = localtime(&t);

                //? Obtener fecha del libro
                strcpy(fecha, ejemplar[i].libroEjem.fecha);
                strptime(fecha, "%d-%m-%Y", fechaLibro);

                //? Añadirle 1 semana
                fechaLibro->tm_sec += WEEK_SEC;
                time_t futura = mktime(fechaLibro);

                double diferencia = difftime(futura, t);
                bool tarde = false;
                if (diferencia < 0)
                {
                    fprintf(stderr, "La fecha de entrega ya había vencido...\n");
                    fprintf(stderr, "Nueva fecha de entrega apartir de esta semana\n");

                    memset(buffer, 0, sizeof(buffer));
                    strcpy(buffer, "(DEVOLUCION TARDE) ");

                    // Fecha a partir de hoy
                    fechaLibro = localtime(&t);
                    fechaLibro->tm_sec += WEEK_SEC;
                    (void)mktime(fechaLibro);

                    tarde = true;
                }

                // Formatear la fecha
                strftime(fecha, TAM_STRING, "%d-%m-%Y", fechaLibro);

                printf("IMPORTANTE: El libro está prestado hasta: %s\n", fecha);

                strcpy(ejemplar[i].libroEjem.fecha, fecha);

                if (!tarde)
                    strcpy(buffer, fecha);
                else
                    strcat(buffer, fecha);

                break;
            }
        }

        // 4. Avisar al cliente
        if (!libroActualizado)
        {
            respuesta = generateReponse(package.client, ERROR, NULL);
            fprintf(stderr, "El libro no está disponible\n");
            if (write(pipeCliente, &respuesta, sizeof(respuesta)) < 0)
            {
                perror("Error");
                return ERROR_SOLICITUD;
            }
            return ERROR_SOLICITUD;
        }
        else
        {
            // Enviar también la fecha
            //? Cambiar el tipo de paquete
            respuesta = generateReponse(package.client, RENOVACION, buffer);

            fprintf(stdout, "Solicitud exitosa (%d)\n", package.client);
            if (write(pipeCliente, &respuesta, sizeof(respuesta)) < 0)
            {
                perror("Error");
                return ERROR_SOLICITUD;
            }
            return SUCCESS_GENERIC;
        }
    }
    break;

    case DEVOLVER: //! Petición de devolución
    {
        // Notificar
        printf("La petición es de tipo: DEVOLVER\n");

        // 1. Buscar el libro
        struct ejemplar libro = package.data.libro;

        bool encontrado = false;
        for (int i = 0; i < MAX_CANT_LIBROS; i++)
        {
            if (ejemplar[i].isbn == libro.isbn &&
                (strcmp(ejemplar[i].nombre, libro.nombre) == 0))
            {
                encontrado = true;
            }
        }

        respuesta = generateReponse(package.client, ERROR, NULL);

        if (!encontrado)
        {
            fprintf(stderr, "El libro no fue encontrado...\n");
            if (write(pipeCliente, &respuesta, sizeof(respuesta)) < 0)
            {
                perror("Error");
                return ERROR_SOLICITUD;
            }
            return ERROR_SOLICITUD;
        }

        // Mostrar notificación
        printf("El libro '%s' fue encontrado\n", libro.nombre);

        // 2. Verificar si está //? OCUPADO
        bool libroActualizado = false;
        for (int i = 0; i < MAX_CANT_LIBROS; i++)
        {
            if (ejemplar[i].isbn == libro.isbn &&
                (strcmp(ejemplar[i].nombre, libro.nombre) == 0) &&
                ejemplar[i].libroEjem.estado == 'P' && //? P de PRESTADO
                ejemplar[i].libroEjem.numero == libro.libroEjem.numero)
            {
                printf("El libro '%s' será actualizado\n", libro.nombre);

                // 3. Modificar el estado del libro

                // Actualizar libro
                // Actualizar su estado
                libroActualizado = true;
                ejemplar[i].libroEjem.estado = 'D'; //? Se pone disponible

                // Actualizar su fecha //? FECHA ACTUAL (Devolución)
                char fecha[TAM_STRING];
                memset(fecha, 0, sizeof(fecha));

                time_t t;
                struct tm *tm;

                t = time(NULL);
                tm = localtime(&t);
                strftime(fecha, TAM_STRING, "%d-%m-%Y", tm);

                //? INFORMACION
                printf("IMPORTANTE: El libro fue devuelto en: %s\n", fecha);

                strcpy(ejemplar[i].libroEjem.fecha, fecha);
                strcpy(buffer, fecha);
                break;
            }
        }

        // 4. Avisar al cliente
        if (!libroActualizado)
        {
            respuesta = generateReponse(package.client, ERROR, NULL);
            fprintf(stderr, "El libro no está disponible\n");
            if (write(pipeCliente, &respuesta, sizeof(respuesta)) < 0)
            {
                perror("Error");
                return ERROR_SOLICITUD;
            }
            return ERROR_SOLICITUD;
        }
        else
        {
            // Enviar también la fecha
            //? Cambiar el tipo de paquete
            respuesta = generateReponse(package.client, DEVOLUCION, buffer);

            fprintf(stdout, "Solicitud exitosa (%d)\n", package.client);
            if (write(pipeCliente, &respuesta, sizeof(respuesta)) < 0)
            {
                perror("Error");
                return ERROR_SOLICITUD;
            }
            return SUCCESS_GENERIC;
        }
    }
    break;

    case BUSCAR: //! Petición de búsqueda
    {
        // Notificar
        printf("La petición es de tipo: BUSCAR\n");

        // 1. Buscar el libro
        struct ejemplar libro = package.data.libro;

        bool encontrado = false;
        for (int i = 0; i < MAX_CANT_LIBROS; i++)
        {
            if (ejemplar[i].isbn == libro.isbn &&
                (strcmp(ejemplar[i].nombre, libro.nombre) == 0))
            {
                encontrado = true;

                respuesta.type = BOOK;
                respuesta.client = package.client;
                respuesta.data.libro = ejemplar[i];

                if (write(pipeCliente, &respuesta, sizeof(respuesta)) < 0)
                {
                    perror("Error");
                    return ERROR_COMUNICACION;
                }

                // Mostrar notificación
                printf("El libro '%s' fue encontrado\n", libro.nombre);

                return SUCCESS_GENERIC;
            }
        }

        respuesta = generateReponse(package.client, ERROR, NULL);
        respuesta.type = ERR;

        fprintf(stderr, "El libro no fue encontrado...\n");
        if (write(pipeCliente, &respuesta, sizeof(respuesta)) < 0)
        {
            perror("Error");
            return ERROR_SOLICITUD;
        }
        return ERROR_SOLICITUD;

        // Mostrar notificación
        fprintf(stderr, "El libro '%s' NO fue encontrado\n", libro.nombre);
    }
    break;

    default:
        return ERROR_COMUNICACION;
    }

    return SUCCESS_GENERIC;
}
