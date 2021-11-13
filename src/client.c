/**
 * @file client.c
 * @authors  Ángel David Talero
 *          Juan Esteban Urquijo
 *          Humberto Rueda Cataño
 * @brief Proceso solicitante
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
#include <sys/time.h>

// Header propias
#include "common.h"
#include "client.h"
#include "paquet.h"
#include "book.h"

/* --------------------------------- Main --------------------------------- */
int main(int argc, char *argv[])
{
    // Manejar los argumentos
    char requestFilename[TAM_STRING], // Nombre archivo
        pipeCLNT_SRVR[TAM_STRING],    // Nombre pipe (Cliente -> Servidor)
        pipeSRVR_CLNT[TAM_STRING];    // Nombre pipe (Servidor -> Cliente)

    //? La declaración de estas variables se hace antes de la validación porque
    //? la función 'manejarArgumentos' necesita una referencia a estas variables
    //? como parámetro

    bool archivoUsado; // Flag para saber si un archivo está siendo usado
    archivoUsado = manejarArgumentos(argc, argv, pipeCLNT_SRVR, requestFilename);

    // Otras variables
    int fd_pipes[2]; // FD's de los pipes

    char nombreLibro[TAM_STRING]; // Buffer para Nombre del libro
    memset(nombreLibro, 0, sizeof(nombreLibro));

    char ISBNstr[TAM_STRING]; // Buffer un ISBN del libro
    memset(ISBNstr, 0, sizeof(ISBNstr));

    // Archivo con las peticiones
    FILE *requestsFile = NULL;

    // Iniciar la comunicación con el servidor
    iniciarComunicacion(pipeCLNT_SRVR, pipeSRVR_CLNT, fd_pipes);

    // Manejar el archivo
    if (archivoUsado)
    {
        // Si utilizó el archivo

        // Abrir el archivo
        requestsFile = fopen(requestFilename, "r");
        if (requestsFile == NULL)
        {
            perror("Archivo");
            return ERROR_APERTURA_ARCHIVO;
        }

        //1. Leer el archivo
        struct peticion_t peticiones[MAX_CANT_LIBROS];
        book_t libro;
        int peticionesLeidas = 0;

        for (int i = 0; i < MAX_CANT_LIBROS; i++)
        {
            fscanf(requestsFile, "%c, %[^,],%d\n",
                   &peticiones[i].request,
                   peticiones[i].bookName,
                   &peticiones[i].ISBN);

            peticionesLeidas++;
            if (feof(requestsFile))
                break;
        }

        // Mandar al servidor todas las peticiones
        for (int i = 0; i < peticionesLeidas; i++)
        {
            switch (peticiones[i].request)
            {
            case 'P': // Prestar
                printf("\nIniciando préstamo del libro\n");
                if (
                    prestarLibro(
                        fd_pipes,
                        peticiones[i].bookName,
                        peticiones[i].ISBN) != SUCCESS_GENERIC)
                    printf("Operación fallida\n");
                else
                    printf("Operación exitosa\n");
                break;

            case 'R': // Renovar
                printf("\nIniciando renovación del libro\n");
                // Hay que intentar renovar por cada libro
                libro = buscarLibro(fd_pipes,
                                    peticiones[i].bookName,
                                    peticiones[i].ISBN);

                if (libro.petition == BUSCAR) // Si el libro no existe
                {
                    fprintf(stderr, "El libro no existe!\n");
                    continue; // Saltarse la peticion
                }

                bool renovado = false;
                for (int j = 0; j < libro.n_copies; j++)
                {
                    printf("Intentando renovar ejemplar #%d", j);
                    // Por cada ejemplar hacer el intento de renovar
                    if (renovarLibro(fd_pipes, libro.name, libro.ISBN, j) ==
                        SUCCESS_GENERIC)
                    {
                        renovado = true; // Al menos un libro fue exitoso
                        break;
                    }
                }

                if (renovado)
                    printf("La renovación del libro fue exitosa...\n");

                else
                    fprintf(stderr, "La renovación del libro falló\n");

                break;

            case 'D': // Devolver un libro
                printf("\nIniciando devolución del libro\n");
                // Hay que intentar renovar por cada libro
                libro = buscarLibro(fd_pipes,
                                    peticiones[i].bookName,
                                    peticiones[i].ISBN);

                if (libro.petition == BUSCAR) // Si el libro no existe
                {
                    fprintf(stderr, "El libro no existe!\n");
                    continue; // Saltarse la peticion
                }

                bool devuelto = false;
                for (int j = 0; j < libro.n_copies; j++)
                {
                    printf("Intentando devolver ejemplar #%d", j);
                    // Por cada ejemplar hacer el intento de renovar
                    if (devolverLibro(fd_pipes, libro.name, libro.ISBN, j) ==
                        SUCCESS_GENERIC)
                    {
                        devuelto = true; // Al menos un libro fue exitoso
                        break;
                    }
                }

                if (devuelto)
                    printf("La devolución del libro fue exitosa...\n");

                else
                    fprintf(stderr, "La devolución del libro falló\n");

                break;

            default:
                break;
            }
        }
    }

    else
    {
        // Si no utilizó el archivo
        //! Mostrar menú

        // Variables del menú
        int sel, n_ejemplar = 0;

        do
        {
            printf("\n @ Menú Principal @\n");
            // Mostrar opciones
            printf("1. Pedir un libro prestado\n");
            printf("2. Renovar un libro\n");
            printf("3. Devolver un libro\n");
            printf("0. Salir\n");
            printf("Seleccione una opción: ");

            scanf("%d", &sel);
            (void)getchar();

            switch (sel)
            {
            case 0:
                // Salir
                printf("Saliendo...\n");
                break;

            case 1:
                // Pedir prestado un libro
                printf("Digite el nombre del libro: ");
                fgets(nombreLibro, sizeof(nombreLibro), stdin);
                nombreLibro[strcspn(nombreLibro, "\r\n")] = 0;

                printf("Digite el ISBN del libro: ");
                fgets(ISBNstr, sizeof(ISBNstr), stdin);

                if (prestarLibro(fd_pipes, nombreLibro, atoi(ISBNstr)) != SUCCESS_GENERIC)
                    printf("Operación fallida\n");
                else
                    printf("Operación exitosa\n");

                break;

            case 2:
                // Renovar un libro
                printf("Digite el nombre del libro: ");
                fgets(nombreLibro, sizeof(nombreLibro), stdin);
                nombreLibro[strcspn(nombreLibro, "\r\n")] = 0;

                printf("Digite el ISBN del libro: ");
                fgets(ISBNstr, sizeof(ISBNstr), stdin);

                printf("Digite el número de ejemplar: ");
                scanf("%d", &n_ejemplar);
                (void)getchar();

                if (renovarLibro(fd_pipes, nombreLibro, atoi(ISBNstr), n_ejemplar) !=
                    SUCCESS_GENERIC)
                    printf("Operación fallida\n");
                else
                    printf("Operación exitosa\n");

                break;

            case 3:
                // Devolver un libro
                printf("Digite el nombre del libro: ");
                fgets(nombreLibro, sizeof(nombreLibro), stdin);
                nombreLibro[strcspn(nombreLibro, "\r\n")] = 0;

                printf("Digite el ISBN del libro: ");
                fgets(ISBNstr, sizeof(ISBNstr), stdin);

                printf("Digite el número de ejemplar: ");
                scanf("%d", &n_ejemplar);
                (void)getchar();

                if (devolverLibro(fd_pipes, nombreLibro, atoi(ISBNstr), n_ejemplar) !=
                    SUCCESS_GENERIC)
                    printf("Operación fallida\n");
                else
                    printf("Operación exitosa\n");

                break;

            default:
                printf("Opción incorrecta...\n");
                break;
            }

        } while (sel != 0);
    }

    // Cerrar archivos abiertos
    if (archivoUsado)
        if (fclose(requestsFile) < 0)
        {
            perror("File");
            return ERROR_CIERRE_ARCHIVO;
        }

    // Cerrar la comunicacion
    detenerComunicacion(fd_pipes, pipeSRVR_CLNT);

    // Notificar
    fprintf(stdout, "\nCliente finaliza correctamente\n");
    return EXIT_SUCCESS;
}

/* ----------------------- Definiciones de funciones ----------------------- */

// Manejo de argumentos

void mostrarUso(void)
{
    fprintf(stderr, "Uso: ./client [-i Archivo] -p NombreDelPipe\n");
    fprintf(stderr, "[-i archivo] es opcional!\n");
    exit(ERROR_ARG_NOVAL);
}

bool manejarArgumentos(int argc, char *argv[], char *pipeNom, char *fileNom)
{
    // Flags para saber si ya se usaron los argumentos -i -p y si el archivo
    // fue abierto con -i

    bool argArchivo = false;
    bool argNombrePipe = false;
    bool archivoUsado = false;

    // Saber cómo reaccionar antes determinado número de argumentos
    switch (argc)
    {
    case 3: // Sólo se usó el argumento -p
        if (argv[1][0] == '-' && argv[1][1] == 'p')
            // Retornar el nombre del pipe (recortado sin el -p)
            strcpy(pipeNom, argv[2]);

        else // Argumentos incorrectos
            mostrarUso();

        break;

    case 5: // Se utilizaron -i y -p

        // Filtrar los argumentos
        while ((argc > 1) && (argv[1][0] == '-'))
        {
            // Por cada flag
            switch (argv[1][1])
            {
            case 'i': // Caso de archivo
                // Verificar si ya se usó el argumento
                if (argArchivo)
                {
                    fprintf(stderr,
                            "El argumento %s ya fue utilizado!\n", argv[1]);
                    mostrarUso();
                }

                // El argumento ya fue utilizado!
                argArchivo = true;
                // Un archivo fue utilizado
                archivoUsado = true;

                // retornar el nombre de archivo
                strcpy(fileNom, argv[2]);

                break;

            case 'p':

                // Verificar si ya se usó el argumento
                if (argNombrePipe)
                {
                    fprintf(stderr,
                            "El argumento %s ya fue utilizado!\n", argv[1]);
                    mostrarUso();
                }

                // El argumento ya fue utilizado!
                argNombrePipe = true;

                // Retornar el nombre del pipe
                strcpy(pipeNom, argv[2]);

                break;

            default:
                fprintf(stderr, "Argumento no válido: %s\n", argv[1]);
                mostrarUso();
            }

            argv += 2; // Mover el puntero de argumentos
            argc -= 2; // Reducir cantidad de argumentos para el while
        }

        break;

    default:
        mostrarUso();
        break;
    }

    return archivoUsado;
}

// Protocolos de comunicación

void iniciarComunicacion(
    const char *pipeCLNT_SRVR,
    char *pipeSRVR_CLNT,
    int *pipe)
{
    // Notificar
    fprintf(stdout, "\n(%d) Intentando establecer conexión\n", getpid());

    // int *pipe Arreglo con los fd de los pipes
    // pipe[WRITE] tiene el pipe de escritura (Cliente -> Servidor)
    // pipe[READ] tiene el pipe de lectura (Servidor -> Cliente)

    //!1 Cliente abre el pipe (Cliente->Servidor) para ESCRITURA

    pipe[WRITE] = open(pipeCLNT_SRVR, O_WRONLY);
    if (pipe[WRITE] < 0)
    {
        perror("Error de comunicación con el servidor"); // Manejar error
        exit(ERROR_PIPE_SRVR_CLNT);
    }

    // Notificar
    fprintf(stdout, "Notificación: El pipe (Cliente->Servidor) fue abierto\n");

    //!2 Cliente crea un pipe (Servidor->Cliente)
    /* Crear el nombre del pipe, para esto se toma el macro PIPE_NOM_CLNT y se
       le concatena el pid del proceso Cliente quien lo crea */

    pid_t client_pid = getpid();

    // 2.1 Borrar los contenidos actuales de pipeSRVR_CLNT
    memset(pipeSRVR_CLNT, 0, sizeof(pipeSRVR_CLNT));

    // 2.2 Copiar la macro a la variable
    strcpy(pipeSRVR_CLNT, PIPE_TITLE_CLNT);

    // 2.3 Concatenar el pid
    char buffer[TAM_STRING];
    sprintf(buffer, "%d", client_pid);
    strcat(pipeSRVR_CLNT, buffer);

    // Ya se tiene el nombre disponible para crear el pipe de LECTURA

    // Crear el pipe nominal
    unlink(pipeSRVR_CLNT);
    if (mkfifo(pipeSRVR_CLNT, PERMISOS_PIPE) < 0)
    {
        perror("Error de conexión con el servidor"); // Manejar Error

        // Cerrar recursos abiertos
        close(pipe[WRITE]);
        exit(ERROR_PIPE_CLNT_SRVR);
    }

    //Notificación
    fprintf(stdout, "Notificación: El pipe (Servidor->Cliente) fue creado\n");

    //!5 Cliente envía a Servidor el nombre del pipe (Servidor->Cliente)
    paquet_t com;                                  // Estructura a enviar por el pipe
    com.client = client_pid;                       // PID quien envía
    com.type = SIGNAL;                             // Tipo de dato
    com.data.signal.code = START_COM;              // Tipo de señal
    strcpy(com.data.signal.buffer, pipeSRVR_CLNT); // Nombre del pipe

    // Intentar enviar los datos
    int aux = 0, attemps = 0;
    do
    {
        aux = write(pipe[WRITE], &com, sizeof(com));

        // Si la escritura falla
        if (aux < 0)
        {
            // Se intenta INTENTOS_ESCRITURA veces
            if (attemps > INTENTOS_ESCRITURA)
            {
                // Muchos intentos fallidos
                perror("Error al escribir");
                fprintf(stderr, "Demasiados intentos, abortando...\n");

                // Cerrar los archivos abiertos
                close(pipe[READ]);
                close(pipe[WRITE]);
                unlink(pipeSRVR_CLNT);

                // Terminar
                exit(ERROR_ESCRITURA);
            }

            perror("Error al escribir");
            fprintf(stderr, "Intentando de nuevo...\n");

            attemps++;
        }

    } while (aux < 0);

    //!4 Cliente abre el pipe (Servidor->Cliente) para LECTURA

    //? Este pipe se abre en modo O_NONBLOCK porque puede
    //? recibir confirmaciones asíncronas

    pipe[READ] = open(pipeSRVR_CLNT, O_RDONLY);
    if (pipe[READ] < 0)
    {
        perror("Error de conexión con el servidor"); // Manejar Error

        // Cerrar recursos abiertos
        close(pipe[WRITE]);
        unlink(pipeSRVR_CLNT);
        exit(ERROR_PIPE_CLNT_SRVR);
    }

    //Notificación
    fprintf(stdout, "Notificación: El pipe (Servidor->Cliente) fue abierto\n");

    //!8. Cliente espera una señal de Servidor

    // Crear un TIMEOUT
    struct timeval start, now;
    time_t elapsedTime;         // Tiempo transcurrido
    gettimeofday(&start, NULL); // Momento inicial

    // Notificación
    fprintf(stdout, "Notificación: Esperando respuesta del Servidor\n");

    paquet_t expect; // Estructura que se espera
    while (read(pipe[READ], &expect, sizeof(expect)) == 0)
    {
        // Obtener el momento actual
        gettimeofday(&now, NULL);
        elapsedTime = now.tv_sec - start.tv_sec;

        if (elapsedTime > TIMEOUT_COMUNICACION)
        {
            fprintf(stderr, "Se superó el tiempo de espera (%ds) ...\
            \nProceso abortado\n",
                    TIMEOUT_COMUNICACION);

            // Cerrar los recursos abiertos
            close(pipe[READ]);
            close(pipe[WRITE]);
            unlink(pipeSRVR_CLNT);

            exit(ERROR_COMUNICACION);
        }
    }

    // Si hay una comunicación fallida
    if (expect.data.signal.code == FAILED_COM)
    {
        // Cerrar los recursos abiertos
        close(pipe[READ]);
        close(pipe[WRITE]);
        unlink(pipeSRVR_CLNT);

        // Terminar el programa
        perror("Comunicacion");
        exit(ERROR_COMUNICACION);
    }

    // Señal de verificación
    else if (expect.data.signal.code == SUCCEED_COM) // Confirmación exitosa
    {
        // Notificación
        fprintf(stdout,
                "Notificación: Comunicación establecida!\n");
        return;
    }

    // Cualquier otro valor es inesperado
    fprintf(stderr, "Respuesta inesperada: %d\n", expect.data.signal.code);
}

static void detenerComunicacion(int *pipe, char *pipeSRVR_CLNT)
{
    // Notificar
    fprintf(stdout, "\n(%d) Intentando detener conexión\n", getpid());

    //!1. Cliente manda una petición de terminación de comunicación al Servidor
    paquet_t packet = generarSenal(getpid(), STOP_COM, NULL);

    // Notificar
    fprintf(stdout, "Intentando cerrar comunicación\n");

    if (write(pipe[WRITE], &packet, sizeof(packet)) < 0)
        perror("Escritura");
    // NO se termina el proceso, para que así elimine el pipe
    // Sólo se intenta escribir indeterminadamente

    paquet_t tmp;
    //!4. Cliente espera a que se cierre el pipe
    while (read(pipe[READ], &tmp, sizeof(tmp)) != 0)
        ;

    // Notificar
    fprintf(stdout, "Esperando que el servidor cierre comunicación\n");

    //!5. Cliente cierra la lectura del pipe (Servidor->Cliente)
    close(pipe[READ]);
    // Notificar
    fprintf(stdout, "Se cerró el pipe (Servidor->Cliente)\n");

    //!6. Cliente elimina el pipe (Servidor->Cliente)
    unlink(pipeSRVR_CLNT);
    // Notificar
    fprintf(stdout, "Se eliminó el pipe (Servidor->Cliente)\n");

    //!7. Cliente cierra la escritura del pipe (Cliente->Servidor)
    close(pipe[WRITE]);
    // Notificar
    fprintf(stdout, "Se cerró el pipe (Cliente->Servidor)\n");

    //!8. El proceso Cliente finaliza
    // Esto se hace desde el main
    // Notificar
    fprintf(stdout, "Comunicación terminada\n");
}

paquet_t generarSenal(pid_t dest, int code, char *buffer)
{
    // Paquet creation
    paquet_t packet;
    packet.client = dest;
    packet.type = SIGNAL;

    // Signal construction
    packet.data.signal.code = code;
    if (buffer != NULL)
        strcpy(packet.data.signal.buffer, buffer);
    return packet;
}

// Manipular libros

int prestarLibro(int *pipes, const char *nombreLibro, int ISBN)
{
    // Notificación
    printf("\nSe está enviando una solicitud al servidor\n");

    // Libro a enviar al servidor
    book_t libro;
    libro.ISBN = ISBN;
    strcpy(libro.name, nombreLibro);
    libro.petition = SOLICITAR;

    // Crear el paquete
    paquet_t paquete;
    paquete.type = BOOK;
    paquete.client = getpid();
    paquete.data.libro = libro;

    // Enviar al sevidor
    if (write(pipes[WRITE], &paquete, sizeof(paquete)) < 0)
    {
        perror("Error");
        return ERROR_ESCRITURA;
    }

    // ... Esperar una respuesta positiva
    paquet_t respuesta;
    if (read(pipes[READ], &respuesta, sizeof(respuesta)) < 0)
    {
        perror("Error");
        return ERROR_LECTURA;
    }

    if (respuesta.data.signal.code != SOLICITUD)
    {
        fprintf(stderr, "La solicitud falló, el libro no existe o no tiene ejemplares disponibles\n");
        return ERROR_SOLICITUD;
    }

    printf("La solicitud fue procesada adecuadamente\n");
    printf("El libro fue prestado hasta: %s\n", respuesta.data.signal.buffer);

    return SUCCESS_GENERIC;
}

int devolverLibro(int *pipes, const char *nombreLibro, int ISBN, int ejemplar)
{
    // Notificación
    printf("\nSe está enviando una solicitud al servidor\n");

    // Libro a enviar al servidor
    book_t libro;
    libro.ISBN = ISBN;
    strcpy(libro.name, nombreLibro);
    libro.petition = DEVOLVER;        //! DEVOLVER
    libro.copyInfo.n_copy = ejemplar; // Numero del ejemplar

    // Crear el paquete
    paquet_t paquete;
    paquete.type = BOOK;
    paquete.client = getpid();
    paquete.data.libro = libro;

    // Enviar al sevidor
    if (write(pipes[WRITE], &paquete, sizeof(paquete)) < 0)
    {
        perror("Error");
        return ERROR_ESCRITURA;
    }

    // ... Esperar una respuesta positiva
    paquet_t respuesta;
    if (read(pipes[READ], &respuesta, sizeof(respuesta)) < 0)
    {
        perror("Error");
        return ERROR_LECTURA;
    }

    if (respuesta.data.signal.code != DEVOLUCION)
    {
        fprintf(stderr, "La solicitud falló, el libro no existe o no tiene ejemplares en préstamo\n");
        return ERROR_SOLICITUD;
    }

    printf("La solicitud fue procesada adecuadamente\n");
    printf("El libro: '%s' fue devuelto correctamente en: %s\n",
           libro.name,
           respuesta.data.signal.buffer);

    return SUCCESS_GENERIC;
}

int renovarLibro(int *pipes,
                 const char *nombreLibro,
                 int ISBN,
                 int ejemplar)
{
    // Notificación
    printf("\nSe está enviando una solicitud al servidor\n");

    // Libro a enviar al servidor
    book_t libro;
    libro.ISBN = ISBN;
    strcpy(libro.name, nombreLibro);
    libro.petition = RENOVAR;         //! DEVOLVER
    libro.copyInfo.n_copy = ejemplar; // Numero del ejemplar

    // Crear el paquete
    paquet_t paquete;
    paquete.type = BOOK;
    paquete.client = getpid();
    paquete.data.libro = libro;

    // Enviar al sevidor
    if (write(pipes[WRITE], &paquete, sizeof(paquete)) < 0)
    {
        perror("Error");
        return ERROR_ESCRITURA;
    }

    // ... Esperar una respuesta positiva
    paquet_t respuesta;
    if (read(pipes[READ], &respuesta, sizeof(respuesta)) < 0)
    {
        perror("Error");
        return ERROR_LECTURA;
    }

    if (respuesta.data.signal.code != RENOVACION)
    {
        fprintf(stderr, "La solicitud falló, el libro no existe o no tiene ejemplares en préstamo\n");
        return ERROR_SOLICITUD;
    }

    printf("La solicitud fue procesada adecuadamente\n");
    printf("El libro '%s' fue renovado\n", libro.name);
    printf("La nueva fecha de entrega es: %s\n",
           respuesta.data.signal.buffer);

    return SUCCESS_GENERIC;
}

book_t buscarLibro(int *pipes, const char *nombre, int ISBN)
{
    // Notificación
    printf("\nSe está enviando una solicitud al servidor\n");

    // Libro a enviar al servidor
    book_t libro1;
    libro1.ISBN = ISBN;
    strcpy(libro1.name, nombre);
    libro1.petition = BUSCAR;

    // Crear el paquete
    paquet_t paquete;
    paquete.type = BOOK;
    paquete.client = getpid();
    paquete.data.libro = libro1;

    // Enviar al sevidor
    if (write(pipes[WRITE], &paquete, sizeof(paquete)) < 0)
    {
        perror("Error");
    }

    // ... Esperar una respuesta positiva
    paquet_t respuesta;
    if (read(pipes[READ], &respuesta, sizeof(respuesta)) < 0)
    {
        perror("Error");
    }

    if (respuesta.type == ERR)
    {
        printf("El libro no existe!");
        return libro1;
    }

    book_t libro;
    libro.ISBN = respuesta.data.libro.ISBN;
    libro.copyInfo = respuesta.data.libro.copyInfo;
    strcpy(libro.name, respuesta.data.libro.name);
    libro.n_copies = respuesta.data.libro.n_copies;
    libro.petition = SOLICITAR;

    printf("La solicitud fue procesada\n\n");
    return libro;
}