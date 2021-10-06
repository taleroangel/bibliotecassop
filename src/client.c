/**
 * @file client.c
 * @author  Ángel David Talero
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
#include "data.h"

/* --------------------------------- Main --------------------------------- */
int main(int argc, char *argv[])
{
    // Manejar los argumentos
    char nombreArchivo[TAM_STRING], // Nombre archivo
        pipeServidor[TAM_STRING],   // Nombre pipe (Cliente -> Servidor)
        pipeCliente[TAM_STRING];    // Nombre pipe (Servidor -> Cliente)

    int pipe[2];       // FD's de los pipes
    bool archivoUsado; // Flag para saber si un archivo está siendo usado

    archivoUsado = manejarArgumentos(argc, argv, pipeServidor, nombreArchivo);
    int archivofd = -1;

    // Iniciar la comunicación con el servidor
    iniciarComunicacion(pipeServidor, pipeCliente, pipe);

    // Manejar el archivo
    if (archivoUsado)
    { // Si utilizó el archivo
        // Abrir el archivo
        archivofd = abrirArchivo(nombreArchivo);
    }

    else
    { // Si NO utilizó el archivo
        // Mostrar el menú
    }

    // Cerrar archivos abiertos
    if (archivoUsado)
        cerrarArchivo(archivofd);

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
                    printf("El argumento %s ya fue utilizado!\n", argv[1]);
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
                    printf("El argumento %s ya fue utilizado!\n", argv[1]);
                    mostrarUso();
                }

                // El argumento ya fue utilizado!
                argNombrePipe = true;

                // Retornar el nombre del pipe
                strcpy(pipeNom, argv[2]);

                break;

            default:
                printf("Argumento no válido: %s\n", argv[1]);
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

// Manejo de archivos

int abrirArchivo(char *nombre)
{
    // Intentar abrir un pipe en modo de sólo lectura
    int fd = open(nombre, O_RDONLY);
    if (fd < 0)
    {
        perror("Error en lectura de archivos");
        exit(ERROR_APERTURA_ARCHIVO);
    }

    return fd;
}

void cerrarArchivo(int fd)
{
    if (close(fd) < 0)
    {
        perror("Error de archivo");
        exit(ERROR_CIERRE_ARCHIVO);
    }
}

// Protocolos de comunicación

void iniciarComunicacion(
    const char *pipeCTE_SER,
    char *pipeSER_CTE,
    int *pipe)
{
    // int *pipe Arreglo con los fd de los pipes
    // pipe[WRITE] tiene el pipe de escritura (Cliente -> Servidor)
    // pipe[READ] tiene el pipe de lectura (Servidor -> Cliente)

    //!1 Cliente abre el pipe (Cliente->Servidor) para ESCRITURA

    setvbuf(stdout, NULL, _IONBF, 0);
    pipe[WRITE] = open(pipeCTE_SER, O_WRONLY | O_NONBLOCK);
    if (pipe[WRITE] < 0)
    {
        perror("Error de comunicación con el servidor"); // Manejar error
        exit(ERROR_PIPE_SER_CTE);
    }

    //!2 Cliente crea un pipe (Servidor->Cliente)
    /* Crear el nombre del pipe, para esto se toma el macro PIPE_NOM_CTE y se
       le concatena el pid del proceso Cliente quien lo crea */

    pid_t client_pid = getpid();

    // 2.1 Borrar los contenidos actuales de pipeSER_CTE
    memset(pipeSER_CTE, 0, sizeof(pipeSER_CTE));

    // 2.2 Copiar la macro a la variable
    strcpy(pipeSER_CTE, PIPE_NOM_CTE);

    // 2.3 Concatenar el pid
    char buffer[TAM_STRING];
    sprintf(buffer, "%d", client_pid);
    strcat(pipeSER_CTE, buffer);

    // Ya se tiene el nombre disponible para crear el pipe de LECTURA

    // Crear el pipe nominal
    unlink(pipeSER_CTE);
    if (mkfifo(pipeSER_CTE, PERMISOS_PIPE) < 0)
    {
        perror("Error de conexión con el servidor"); // Manejar Error

        // Cerrar recursos abiertos
        close(pipe[WRITE]);
        exit(ERROR_PIPE_CTE_SER);
    }

    setvbuf(stdout, NULL, _IONBF, 0);

    //!3 Cliente abre el pipe (Servidor->Cliente) para LECTURA
    pipe[READ] = open(pipeSER_CTE, O_RDONLY);
    if (pipe[READ] < 0)
    {
        perror("Error de conexión con el servidor"); // Manejar Error

        // Cerrar recursos abiertos
        close(pipe[WRITE]);
        exit(ERROR_PIPE_CTE_SER);
    }

    // TEST
    printf("TEST");

    //!4 Cliente envía a Servidor el nombre del pipe (Servidor->Cliente)
    data_t com;                                  // Estructura a enviar por el pipe
    com.client = client_pid;                     // PID quien envía
    com.type = SIGNAL;                           // Tipo de dato
    com.data.signal.code = START_COM;            // Tipo de señal
    strcpy(com.data.signal.buffer, pipeSER_CTE); // Nombre del pipe

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

                // Terminar
                exit(ERROR_ESCRITURA);
            }

            perror("Error al escribir");
            printf("Intentando de nuevo...\n");
            attemps++;
        }

    } while (aux < 0);

    //!8. Cliente espera una señal de Servidor

    // Crear un TIMEOUT
    struct timeval start, now;
    time_t elapsedTime;         // Tiempo transcurrido
    gettimeofday(&start, NULL); // Momento inicial

    // TEST
    printf("TEST: %d", start.tv_sec);

    data_t expect; // Estructura que se espera
    while (read(pipe[READ], &expect, sizeof(expect)) == 0)
    {
        // Obtener el momento actual
        gettimeofday(&now, NULL);
        elapsedTime = now.tv_sec - start.tv_sec;

        if (elapsedTime > TIMEOUT_COMUNICACION)
        {
            perror("Error");
            fprintf(stderr, "Se superó el tiempo de espera(%ds) ...",
                    TIMEOUT_COMUNICACION);

            // Cerrar los recursos abiertos
            close(pipe[READ]);
            close(pipe[WRITE]);
            exit(ERROR_COMUNICACION);
        }
    }

    // Si hay una comunicación fallida
    if (expect.data.signal.code == FAILED_COM)
    {
        // Cerrar los recursos abiertos
        close(pipe[READ]);
        close(pipe[WRITE]);

        // Terminar el programa
        perror("Comunicacion");
        exit(ERROR_COMUNICACION);
    }

    // Señal de verificación
    else if (expect.data.signal.code == SUCCEED_COM) // Confirmación exitosa
        return;

    // Cualquier otro valor es inesperado
    fprintf(stderr, "Respuesta inesperada: %d\n", expect.data.signal.code);
}
