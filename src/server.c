/**
 * @file server.c
 * @authors  Ángel David Talero
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
#define _POSIX_C_SOURCE 200809L

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
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>

// Header propias
#include "server.h"
#include "common.h"
#include "paquet.h"
#include "book.h"
#include "buffer.h"

/* -------------------- Variables globales (Semáforos) -------------------- */

sem_t semaforo_bd = {0};
sem_t semaforo_clientes = {0};

volatile bool isListening = true;

/* --------------------------------- Main --------------------------------- */
int main(int argc, char *argv[])
{
    char pipeCLNT_SRVR[TAM_STRING],
        inputFilename[TAM_STRING],
        outputFilename[TAM_STRING];

    //! 1. Manejar los argumentos
    // 1.1 Cargar los argumentos
    manejarArgumentos(argc, argv, pipeCLNT_SRVR, inputFilename, outputFilename);

    // 1.2 Verificar que el archivo de persistencia existe, si no existe, crearlo
    if (access(outputFilename, F_OK) != 0)
    {
        fprintf(stderr, "El archivo espeficifado de persistencia no existe...\n");
        fprintf(stdout, "Creando el archivo\n");

        // Crear el archivo
        int tmp_fd = open(outputFilename, O_WRONLY | O_CREAT, 0777);
        if (tmp_fd < 0)
        {
            perror(outputFilename);
            exit(ERROR_APERTURA_ARCHIVO);
        }

        if (close(tmp_fd) < 0)
        {
            perror(outputFilename);
            exit(ERROR_CIERRE_ARCHIVO);
        }
    }

    //! 2. Base de datos
    // 2.1 Crear el arreglo de base de datos
    book_t booksDatabase[MAX_CANT_LIBROS];
    // 2.2 Abrir la base de datos
    int n_libros = leerDatabase(booksDatabase, inputFilename);

    //! 3. Iniciar la comunicación (Escuchar a cualquier cliente)
    int readPipe = iniciarComunicacion(pipeCLNT_SRVR);

    // 3.2 Crear lista de los clientes (Memoria Dinámica)
    struct client_list clients;
    clients.n_clients = 0;
    clients.clientArray = (client_t *)malloc(sizeof(client_t));

    // 3.3 Paquete temporal donde se guarda lo recibido por el pipe
    paquet_t package;

    //! 4. Buffer interno con las peticiones
    // Crear el buffer intero
    buffer_t buffer_interno;
    init(&buffer_interno);

    //! 5. Crear los semáforos para exclusión mutua
    // 5.1 Crear el semáforo para garantizar exclusión mutua en la BD
    if (sem_init(&semaforo_bd, 0, 1))
    {
        perror("Semaforo");
        // Liberar los recursos y salir
        free(clients.clientArray);
        close(readPipe);
        unlink(pipeCLNT_SRVR);
        exit(ERROR_FATAL);
    }

    //  5.2 Crear el semáforo para garantizar exclusión mutua en los clientes
    if (sem_init(&semaforo_clientes, 0, 1))
    {
        perror("Semaforo");
        // Liberar los recursos y salir
        free(clients.clientArray);
        close(readPipe);
        unlink(pipeCLNT_SRVR);
        exit(ERROR_FATAL);
    }

    //! 6. Llamar al hilo auxiliar
    //6.1 Crear la estuctura con los parámetros
    struct arg_buffer parametros_buffer;
    parametros_buffer.booksDatabase = booksDatabase;
    parametros_buffer.buffer = &buffer_interno;
    parametros_buffer.clients = &clients;

    // 6.2 Crear el hilo auxiliar
    pthread_t hilo_aux;
    pthread_create(&hilo_aux, NULL, (void *)manejadorBuffer, (void *)&parametros_buffer);

    // 6.3 Cargar el manejador de señales
    signal(SIGINT, manejadorInterrupcion);

    //! 7. Empezar a escuchar peticiones
    // Leer contenidos del pipe continuamente hasta que no hayan lectores
    bool messageShown = false;
    while (isListening)
    {
        // 7.1 Leer del pipe
        int read_val = read(readPipe, &package, sizeof(package));

        if (read_val == 0)
        {
            if (!messageShown)
            {
                fprintf(stdout, "\n\aTodos los clientes se han desconectado\n");
                fprintf(stdout, "Esperando otras conexiones... (Presione Ctrl+C para detener)\n");
                messageShown = true;
            }
            continue; // Saltar el queue
        }

        messageShown = false;
        // Montar la petición al arreglo de peticiones
        queue(&buffer_interno, package);
    }

    //! 8. Cierre (Liberación de recursos)
    // Notificación
    fprintf(stdout,
            "\nTodos los clientes se han desconectado, cerrando el servidor...\n");

    // Eliminar lista de clientes
    free(clients.clientArray);

    // Deshacer el pipe de Servidor
    close(readPipe);
    unlink(pipeCLNT_SRVR);

    // Unir el thread
    pthread_kill(hilo_aux, SIGUSR1); // Mandar la misma interrupción al hilo
    pthread_join(hilo_aux, (void **)NULL);

    // Liberar el semáforo
    sem_destroy(&semaforo_bd);
    sem_destroy(&semaforo_clientes);

    // Liberar el buffer interno
    destroy(&buffer_interno);

    // Notificación
    fprintf(stdout,
            "Se ha cerrado el pipe (Cliente->Servidor)...\n");

    //* El archivo de entrada ya está cerrado, no hace falta cerrarlo*/

    //! 9. Cierre (Actualización final a la BD)
    // Actualizar la BD (Persistencia de la BD)
    if (actualizarDatabase(outputFilename, booksDatabase, n_libros))
    {
        fprintf(stderr,
                "Hubo un error en el archivo de persistencia de la BD,\
se reintentará la escritura al archivo..\n");

        if (actualizarDatabase(outputFilename, booksDatabase, n_libros))
        {
            fprintf(stderr,
                    "El archivo de la base de datos puede estar dañado,\
los cambios a la BD se mostrarán por pantalla:\n");

            mostrarDatabasePantalla(booksDatabase, n_libros);
        }
    }

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
            "Uso: ./server -p pipeReceptor -f dataBase(Entrada)\n -s dataBase(Salida)");
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

void manejadorInterrupcion(int foo)
{
    printf("\nAVISO: Se ha detenido la ejecución\n");
    isListening = false;
}

/* ----------------------- Manejo de la Base de Datos ----------------------- */

int leerDatabase(book_t booksDatabase[], const char filename[])
{
    // Abrir el archivo para sólo lectura
    FILE *databaseInput = fopen(filename, "r");
    if (databaseInput == NULL)
    {
        perror("Server");
        fprintf(stderr, "Archivo: %s\n", filename);
        exit(ERROR_APERTURA_ARCHIVO);
    }

    // Leer cada libro de la DB
    int n_libro = 0;
    for (int i = 0; i < MAX_CANT_LIBROS; i++)
    {
        if (feof(databaseInput))
            break;

        fscanf(databaseInput, "%[^,],%d,%d\n",
               booksDatabase[n_libro].name,
               &booksDatabase[n_libro].ISBN,
               &booksDatabase[n_libro].n_copies);

        int aux = booksDatabase[n_libro].n_copies;

        for (int j = 0; j < aux; j++)
        {
            if (j > 0)
            {
                strcpy(booksDatabase[n_libro].name, booksDatabase[n_libro - 1].name);
                booksDatabase[n_libro].ISBN = booksDatabase[n_libro - 1].ISBN;
                booksDatabase[n_libro].n_copies = booksDatabase[n_libro - 1].n_copies;
            }

            fscanf(databaseInput, "%d,%c,%s\n",
                   &booksDatabase[n_libro].copyInfo.n_copy,
                   &booksDatabase[n_libro].copyInfo.state,
                   booksDatabase[n_libro].copyInfo.date);

            n_libro++;
        }
    }

    // Mostrar una notificación
    printf("Database: %d ejemplares fueron importados correctamente!\n", n_libro);
    return n_libro;
}

int actualizarDatabase(const char filename[],
                       book_t booksDatabase[],
                       int tam_database)
{
    // 1. Abrir el archivo y validar la syscall
    FILE *database = fopen(filename, "w");
    if (database == NULL)
    {
        perror("Server");
        fprintf(stderr, "Archivo: %s\n", filename);
        exit(ERROR_APERTURA_ARCHIVO);
    }

    // 2. Escribir BD al archivo

    char buffer[TAM_STRING];
    memset(buffer, 0, sizeof(buffer)); // Initialize buffer in 0

    for (int i = 0; i < tam_database; i++)
    {
        // Check if header printing is necessary
        if (strcmp(buffer, booksDatabase[i].name) != 0)
        {
            // Header printing is required
            fprintf(database, "%s,%d,%d\n",
                    booksDatabase[i].name, booksDatabase[i].ISBN,
                    booksDatabase[i].n_copies);
        }

        // Print copy
        fprintf(database, "%d,%c,%s",
                booksDatabase[i].copyInfo.n_copy,
                booksDatabase[i].copyInfo.state,
                booksDatabase[i].copyInfo.date);

        // Print endl
        if (i < tam_database - 1)
            fprintf(database, "\n");

        // Save the buffer
        strcpy(buffer, booksDatabase[i].name);
    }

    // 3. Cerrar la bd

    if (fclose(database) < 0)
    {
        perror("Database");
        return ERROR_CIERRE_ARCHIVO;
    }

    fprintf(stdout, "Database: Actualización satisfactoria\n");
    return SUCCESS_GENERIC;
}

void mostrarDatabasePantalla(book_t booksDatabase[], int tam_database)
{
    fprintf(stdout, "\nBASE DE DATOS:\n");

    char buffer[TAM_STRING];
    memset(buffer, 0, sizeof(buffer)); // Initialize buffer in 0

    for (int i = 0; i < tam_database; i++)
    {
        // Check if header printing is necessary
        if (strcmp(buffer, booksDatabase[i].name) != 0)
        {
            // Header printing is required
            fprintf(stdout, "%s,%d,%d\n",
                    booksDatabase[i].name, booksDatabase[i].ISBN,
                    booksDatabase[i].n_copies);
        }

        // Print copy
        fprintf(stdout, "%d,%c,%s",
                booksDatabase[i].copyInfo.n_copy,
                booksDatabase[i].copyInfo.state,
                booksDatabase[i].copyInfo.date);

        // Print endl
        if (i < tam_database - 1)
            fprintf(stdout, "\n");

        // Save the buffer
        strcpy(buffer, booksDatabase[i].name);
    }

    fprintf(stdout, "\nFIN DE BASE DE DATOS\n\b");
}

/* ----------------------- Protocolos de comunicación ----------------------- */

int iniciarComunicacion(const char *pipeCLNT_SRVR)
{
    // Crear el pipe (Cliente -> Servidor)

    unlink(pipeCLNT_SRVR);
    if (mkfifo(pipeCLNT_SRVR, PERMISOS_PIPE) < 0)
    {
        perror("Error de comunicación"); // Manejar Error
        exit(ERROR_PIPE_SRVR_CLNT);
    }

    // Notificación
    fprintf(stdout, "Notificación: Se ha creado el pipe (Cliente->Servidor)\n");
    fprintf(stdout, "Notificación: El servidor está en estado de espera...\n");

    // Abrir el pipe para lectura (Cliente->Servidor)
    int pipe = open(pipeCLNT_SRVR, O_RDONLY);
    if (pipe < 0)
    {
        perror("Error de comunicación"); // Manejar Error
        exit(ERROR_PIPE_CLNT_SRVR);
    }

    // Notificación
    fprintf(stdout, "Notificación: Se ha abierto el pipe (Cliente->Servidor)\n");

    return pipe;
}

int conectarCliente(struct client_list *clients, paquet_t package)
{
    // Notificación
    fprintf(stdout, "\nUn nuevo cliente está iniciando una conexión\n");

    if (package.type != SIGNAL && package.data.signal.code != START_COM)
    {
        fprintf(stderr, "UnexpeCLNTd instruction\n");
        return ERROR_COMUNICACION;
    }

    //!5. Servidor abre el pipe (Servidor->Cliente) para ESCRITURA
    //Try to open the pipe
    int pipefd = open(package.data.signal.buffer, O_WRONLY);
    if (pipefd < 0)
    {
        perror("Error en comunicación");
        return ERROR_PIPE_SRVR_CLNT;
    }

    // Notificación
    fprintf(stdout,
            "Notificación: El pipe (Servidor->Cliente) fue abierto!: %s\n",
            package.data.signal.buffer);

    //!6. Servidor guarda la información de Cliente con su respectivo pipe
    //!de comunicación

    // Leer los datos en el paquete y convertirlo en cliente
    client_t nuevo = crearCliente(
        pipefd, package.client, package.data.signal.buffer);

    // Guardar el nuevo cliente
    if (guardarCliente(clients, nuevo) != SUCCESS_GENERIC)
        return ERROR_MEMORY;

    // Notificación
    fprintf(stdout,
            "Notificación: Nuevo cliente agregado\n");

    //!7. Servidor envía una señal de confirmación a Cliente

    // Crear una señal
    paquet_t toSent;
    toSent.type = SIGNAL;
    toSent.client = nuevo.clientPID;
    toSent.data.signal.code = SUCCEED_COM;

    // En caso de que el pipe se cierre justo en el envío de la señal
    if (write(nuevo.pipe, &toSent, sizeof(toSent)) < 0)
    {
        perror("Error");
        fprintf(stderr, "Pipe cerrado inesperadamente");

        // Desalojar recursos
        removerCliente(clients, nuevo.clientPID);
        close(pipefd);

        return ERROR_PIPE_SRVR_CLNT;
    }

    // Notificación
    fprintf(stdout,
            "Notificación: Señal enviada\n");

    fprintf(stdout,
            "La comunicación fue exitosa\n");

    return SUCCESS_GENERIC;
}

int retirarCliente(struct client_list *clients, paquet_t package)
{
    // Notificación
    fprintf(stdout, "\nEl cliente (%d) quiere abandonar la comunicación\n",
            package.client);

    if (package.type != SIGNAL && package.data.signal.code != STOP_COM)
    {
        fprintf(stderr, "UnexpeCLNTd instruction\n");
        return ERROR_COMUNICACION;
    }

    pid_t client = package.client;
    int pipeSRVR_CLNT = buscarCliente(clients, client);

    //! 2. Servidor cierra la escritura del pipe (Servidor->Cliente)
    if (close(pipeSRVR_CLNT) < 0)
    {
        perror("Error");
        return ERROR_PIPE_SRVR_CLNT;
    }

    //!3. Servidor actualiza la lista de clientes
    int temp = removerCliente(clients, client);
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

int interpretarSenal(struct client_list *clients, paquet_t package)
{
    switch (package.data.signal.code)
    {
    case START_COM:
        return conectarCliente(clients, package);
        break;

    case STOP_COM:
        return retirarCliente(clients, package);
        break;

    default:
        return FAILED_COM;
    }

    return FAILURE_GENERIC;
}

paquet_t generarRespuesta(pid_t dest, int code, char *buffer)
{
    // Paquet creation
    paquet_t reponse;
    reponse.client = dest;
    reponse.type = SIGNAL;

    // Signal construction
    reponse.data.signal.code = code;
    if (buffer != NULL)
        strcpy(reponse.data.signal.buffer, buffer);

    return reponse;
}

/* --------------------------- Manejo de clientes --------------------------- */

client_t crearCliente(int pipefd, pid_t clientpid, char *pipenom)
{
    client_t clienteNuevo;
    clienteNuevo.clientPID = clientpid;
    clienteNuevo.pipe = pipefd;
    strcpy(clienteNuevo.pipeFilename, pipenom);
    return clienteNuevo;
}

int guardarCliente(struct client_list *clients, client_t client)
{
    //! Esta función es una región crítica
    sem_wait(&semaforo_clientes);

    // Add 1 to the client counter
    int pos_newClient = clients->n_clients++;

    // Realloc memory for the new client
    client_t *aux = // Hacer el vector más grande y guardarlo en un nuevo apuntador
        (client_t *)realloc(
            clients->clientArray,
            sizeof(client_t) * clients->n_clients);

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

    //! Fin de la región crítica
    sem_post(&semaforo_clientes);
    return SUCCESS_GENERIC;
}

int removerCliente(struct client_list *clients, pid_t clientToRemove)
{
    //! Esta función es una región crítica
    sem_wait(&semaforo_clientes);

    // Search for the client to remove and move it to the last position
    if (clients->clientArray[clients->n_clients - 1].clientPID != clientToRemove)
    {
        bool found = false;
        for (int i = 0; i < clients->n_clients; i++)
        {
            if (clients->clientArray[i].clientPID == clientToRemove)
            {
                found = true;

                // Set variables
                client_t toRemove = clients->clientArray[i];
                client_t last = clients->clientArray[clients->n_clients - 1];

                // Swap
                clients->clientArray[i] = last;
                clients->clientArray[clients->n_clients - 1] = toRemove;

                break;
            }
        }

        if (!found) // If PID was not found
            return ERROR_PID_NOT_EXIST;
    }

    // Realloc the array
    if (clients->n_clients > 1)
        clients->n_clients--;

    // Realloc memory for the new client
    client_t *aux = // Hacer el vector más grande y guardarlo en un nuevo apuntador
        (client_t *)realloc(
            clients->clientArray,
            sizeof(client_t) * clients->n_clients);

    // If allocation failed
    if (aux == NULL)
    {
        perror("Error");
        fprintf(stderr, "No es posible eliminar un cliente...\n");
        return ERROR_MEMORY;
    }

    // Set the new pointer
    clients->clientArray = aux;

    //! Fin de la región crítica
    sem_post(&semaforo_clientes);
    return SUCCESS_GENERIC;
}

int buscarCliente(struct client_list *clients, pid_t client)
{
    for (int i = 0; i < clients->n_clients; i++)
        if (clients->clientArray[i].clientPID == client)
            return clients->clientArray[i].pipe;

    return ERROR_PID_NOT_EXIST;
}

/* ---------------------------- Manejo de libros ---------------------------- */

int manejarLibros(
    struct client_list *clients,
    paquet_t package,
    book_t ejemplar[])
{
    // Notificación
    printf("\nSe recibió una solicitud del cliente (%d)\n", package.client);

    // Optener el pipe del cliente
    int pipeCliente = buscarCliente(clients, package.client);

    if (pipeCliente < 0)
    {
        perror("Error");
        return ERROR_COMUNICACION;
    }

    paquet_t respuesta;
    char buffer[TAM_STRING];

    switch (package.data.libro.petition)
    {
    case SOLICITAR: //! Petición de solicitud
    {
        // Notificar
        printf("La petición es de tipo: SOLICITAR\n");

        // 1. Buscar el libro
        book_t libro = package.data.libro;

        bool encontrado = false;
        for (int i = 0; i < MAX_CANT_LIBROS; i++)
        {
            if (ejemplar[i].ISBN == libro.ISBN &&
                (strcmp(ejemplar[i].name, libro.name) == 0))
            {
                encontrado = true;
            }
        }

        respuesta = generarRespuesta(package.client, PET_ERROR, NULL);

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
        printf("El libro '%s' fue encontrado\n", libro.name);

        // 2. Verificar si está disponible

        bool libroActualizado = false;
        for (int i = 0; i < MAX_CANT_LIBROS; i++)
        {
            if (ejemplar[i].ISBN == libro.ISBN &&
                (strcmp(ejemplar[i].name, libro.name) == 0) &&
                ejemplar[i].copyInfo.state == 'D')
            {
                printf("El libro '%s' será actualizado\n", libro.name);

                // 3. Modificar el estado del libro

                // Actualizar libro
                // Actualizar su estado
                libroActualizado = true;
                ejemplar[i].copyInfo.state = 'P';

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

                strcpy(ejemplar[i].copyInfo.date, fecha);
                strcpy(buffer, fecha);

                // Añadir qué ejemplar fue el que se prestó
                //ADVERTENCIA: como ya no se necesita fecha la usamos de string auxiliar
                memset(fecha, 0, sizeof(fecha));
                sprintf(fecha, " (Ejemplar #%d)", ejemplar[i].copyInfo.n_copy);
                strcat(buffer, fecha);

                break;
            }
        }

        // 4. Avisar al cliente
        if (!libroActualizado)
        {
            respuesta = generarRespuesta(package.client, PET_ERROR, NULL);
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
            respuesta = generarRespuesta(package.client, SOLICITUD, buffer);

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
        book_t libro = package.data.libro;

        bool encontrado = false;
        for (int i = 0; i < MAX_CANT_LIBROS; i++)
        {
            if (ejemplar[i].ISBN == libro.ISBN &&
                (strcmp(ejemplar[i].name, libro.name) == 0))
            {
                encontrado = true;
            }
        }

        respuesta = generarRespuesta(package.client, PET_ERROR, NULL);

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
        printf("El libro '%s' fue encontrado\n", libro.name);

        // 2. Verificar si está //? OCUPADO
        bool libroActualizado = false;
        for (int i = 0; i < MAX_CANT_LIBROS; i++)
        {
            if (ejemplar[i].ISBN == libro.ISBN &&
                (strcmp(ejemplar[i].name, libro.name) == 0) &&
                ejemplar[i].copyInfo.state == 'P' && //? P de PRESTADO
                ejemplar[i].copyInfo.n_copy == libro.copyInfo.n_copy)
            {
                printf("El libro '%s' será actualizado\n", libro.name);

                // 3. Modificar el estado del libro

                // Actualizar libro
                // Actualizar su estado
                libroActualizado = true;
                ejemplar[i].copyInfo.state = 'P'; //? Se deja en PRESTADO

                // Actualizar su fecha
                //! A LA FECHA DE DEVOLUCIÓN QUE SE TENÍA se le suma 1 semana

                char fecha[TAM_STRING];
                memset(fecha, 0, sizeof(fecha));

                time_t t;
                struct tm *fechaLibro;

                t = time(NULL);
                fechaLibro = localtime(&t);

                //? Obtener fecha del libro
                strcpy(fecha, ejemplar[i].copyInfo.date);
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

                strcpy(ejemplar[i].copyInfo.date, fecha);

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
            respuesta = generarRespuesta(package.client, PET_ERROR, NULL);
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
            respuesta = generarRespuesta(package.client, RENOVACION, buffer);

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
        book_t libro = package.data.libro;

        bool encontrado = false;
        for (int i = 0; i < MAX_CANT_LIBROS; i++)
        {
            if (ejemplar[i].ISBN == libro.ISBN &&
                (strcmp(ejemplar[i].name, libro.name) == 0))
            {
                encontrado = true;
            }
        }

        respuesta = generarRespuesta(package.client, PET_ERROR, NULL);

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
        printf("El libro '%s' fue encontrado\n", libro.name);

        // 2. Verificar si está //? OCUPADO
        bool libroActualizado = false;
        for (int i = 0; i < MAX_CANT_LIBROS; i++)
        {
            if (ejemplar[i].ISBN == libro.ISBN &&
                (strcmp(ejemplar[i].name, libro.name) == 0) &&
                ejemplar[i].copyInfo.state == 'P' && //? P de PRESTADO
                ejemplar[i].copyInfo.n_copy == libro.copyInfo.n_copy)
            {
                printf("El libro '%s' será actualizado\n", libro.name);

                // 3. Modificar el estado del libro

                // Actualizar libro
                // Actualizar su estado
                libroActualizado = true;
                ejemplar[i].copyInfo.state = 'D'; //? Se pone disponible

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

                strcpy(ejemplar[i].copyInfo.date, fecha);
                strcpy(buffer, fecha);
                break;
            }
        }

        // 4. Avisar al cliente
        if (!libroActualizado)
        {
            respuesta = generarRespuesta(package.client, PET_ERROR, NULL);
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
            respuesta = generarRespuesta(package.client, DEVOLUCION, buffer);

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
        book_t libro = package.data.libro;

        bool encontrado = false;
        for (int i = 0; i < MAX_CANT_LIBROS; i++)
        {
            if (ejemplar[i].ISBN == libro.ISBN &&
                (strcmp(ejemplar[i].name, libro.name) == 0))
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
                printf("El libro '%s' fue encontrado\n", libro.name);

                return SUCCESS_GENERIC;
            }
        }

        respuesta = generarRespuesta(package.client, PET_ERROR, NULL);
        respuesta.type = ERR;

        fprintf(stderr, "El libro no fue encontrado...\n");
        if (write(pipeCliente, &respuesta, sizeof(respuesta)) < 0)
        {
            perror("Error");
            return ERROR_SOLICITUD;
        }
        return ERROR_SOLICITUD;

        // Mostrar notificación
        fprintf(stderr, "El libro '%s' NO fue encontrado\n", libro.name);
    }
    break;

    default:
        return ERROR_COMUNICACION;
    }

    return SUCCESS_GENERIC;
}

void *manejadorBuffer(struct arg_buffer *params)
{
    //! 1. Desempaquetar los parámetros y guardarlos en variables más sencillas
    buffer_t *buffer = params->buffer;
    struct client_list *clients = params->clients;
    book_t *booksDatabase = params->booksDatabase;

    // Activar el manejador de señales
    signal(SIGUSR1, manejadorInterrupcion);

    // Esto ocurre hasta que el padre detenga al hilo
    while (isListening)
    {

        //! 2. Obtener el paquete
        //* Si no hay paquete disponibles el hilo se BLOQUEA por un semáforo*/
        paquet_t *package = getNext(buffer);

        if (!isListening)
        {
            printf("\n\aHilo auxiliar: Terminando ejecución...\n");
            break;
        }

        // Mostrar una notificación
        printf("\nHilo auxiliar: Procesando petición...\n");

        //! 3. Interpretar el paquete
        int return_status = 0;
        switch (package->type)
        {
        case SIGNAL: //* Cuando se recibe una SEÑAL*/

            /* Hay una región crítica en esta parte pero se implementa dentro
                de la misma función encargada de conectar y desconectar clientes*/
            return_status = interpretarSenal(clients, *package);
            if (return_status != SUCCESS_GENERIC)
            {
                fprintf(stderr,
                        "Hubo un problema en la solicitud del cliente (%d)\n",
                        package->client);
                fprintf(stderr, "SEÑAL: Código de error: %d\n", return_status);
            }
            break;

        case BOOK: //* Cuando se recibe un LIBRO*/

            //! Entrando en una región crítica (Base de datos)
            sem_wait(&semaforo_bd);

            return_status = manejarLibros(clients, *package, booksDatabase);
            if (return_status != SUCCESS_GENERIC)
            {
                fprintf(stderr,
                        "Hubo un problema en la solicitud del cliente (%d)\n",
                        package->client);
                fprintf(stderr, "LIBRO: Código de error: %d\n", return_status);
            }

            //! Saliendo de la región crítica
            sem_post(&semaforo_bd);
            break;

        case ERR: //* Cualquier otro caso o error*/
        default:
            fprintf(stderr, "Hubo un problema en la solicitud del cliente (%d)\n",
                    package->client);
            break;
        }

        //! 4. Sacar a la petición de la lista
        dequeue(buffer);
    }

    return NULL; // No hace falta retornar nada
}
