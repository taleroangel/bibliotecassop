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

//temporal
#include <signal.h>
volatile bool runrun = true;
void inttemporal(int dumm)
{
    runrun = false;
}
//endtemporal

/* --------------------------------- Main --------------------------------- */
int main(int argc, char *argv[])
{
    // Manejar los argumentos
    char nombreArchivo[TAM_STRING], // Nombre archivo
        pipeCTE_SER[TAM_STRING],    // Nombre pipe (Cliente -> Servidor)
        pipeSER_CTE[TAM_STRING];    // Nombre pipe (Servidor -> Cliente)

    int pipe[2];       // FD's de los pipes
    bool archivoUsado; // Flag para saber si un archivo está siendo usado

    archivoUsado = manejarArgumentos(argc, argv, pipeCTE_SER, nombreArchivo);
    int archivofd = -1;

    // Iniciar la comunicación con el servidor
    startCommunication(pipeCTE_SER, pipeSER_CTE, pipe);

    // Manejar el archivo
    if (archivoUsado)
    { // Si utilizó el archivo
        // Abrir el archivo
        archivofd = open(nombreArchivo, O_RDWR);
        if (archivofd < 0)
        {
            perror("Archivo");
            return ERROR_APERTURA_ARCHIVO;
        }

        //TODO hacer quien sabe que con ese archivo
    }

    else
    { // Si NO utilizó el archivo
        // Mostrar el menú
    }

    //temporal
    signal(SIGINT, inttemporal);
    while (runrun)
        ;
    //endtemporal

    // Cerrar archivos abiertos
    if (archivoUsado)
        if (close(archivofd) < 0)
            return ERROR_CIERRE_ARCHIVO;

    // Cerrar la comunicacion
    stopCommunication(pipe, pipeSER_CTE);

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

void startCommunication(
    const char *pipeCTE_SER,
    char *pipeSER_CTE,
    int *pipe)
{
    // Notificar
    fprintf(stdout, "\n(%d) Intentando establecer conexión\n", getpid());

    // int *pipe Arreglo con los fd de los pipes
    // pipe[WRITE] tiene el pipe de escritura (Cliente -> Servidor)
    // pipe[READ] tiene el pipe de lectura (Servidor -> Cliente)

    //!1 Cliente abre el pipe (Cliente->Servidor) para ESCRITURA

    pipe[WRITE] = open(pipeCTE_SER, O_WRONLY);
    if (pipe[WRITE] < 0)
    {
        perror("Error de comunicación con el servidor"); // Manejar error
        exit(ERROR_PIPE_SER_CTE);
    }

    // Notificar
    fprintf(stdout, "Notificación: El pipe (Cliente->Servidor) fue abierto\n");

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

    //Notificación
    fprintf(stdout, "Notificación: El pipe (Servidor->Cliente) fue creado\n");

    //!5 Cliente envía a Servidor el nombre del pipe (Servidor->Cliente)
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
                unlink(pipeSER_CTE);

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

    pipe[READ] = open(pipeSER_CTE, O_RDONLY);
    if (pipe[READ] < 0)
    {
        perror("Error de conexión con el servidor"); // Manejar Error

        // Cerrar recursos abiertos
        close(pipe[WRITE]);
        unlink(pipeSER_CTE);
        exit(ERROR_PIPE_CTE_SER);
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

    data_t expect; // Estructura que se espera
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
            unlink(pipeSER_CTE);

            exit(ERROR_COMUNICACION);
        }
    }

    // Si hay una comunicación fallida
    if (expect.data.signal.code == FAILED_COM)
    {
        // Cerrar los recursos abiertos
        close(pipe[READ]);
        close(pipe[WRITE]);
        unlink(pipeSER_CTE);

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

static void stopCommunication(int *pipe, char *pipeSER_CTE)
{
    // Notificar
    fprintf(stdout, "\n(%d) Intentando detener conexión\n", getpid());

    //!1. Cliente manda una petición de terminación de comunicación al Servidor
    data_t packet = generateSignal(getpid(), STOP_COM, NULL);

    // Notificar
    fprintf(stdout, "Intentando cerrar comunicación\n");

    if (write(pipe[WRITE], &packet, sizeof(packet)) < 0)
        perror("Escritura");
    // NO se termina el proceso, para que así elimine el pipe
    // Sólo se intenta escribir indeterminadamente

    data_t tmp;
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
    unlink(pipeSER_CTE);
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

data_t generateSignal(pid_t dest, int code, char *buffer)
{
    // Paquet creation
    data_t packet;
    packet.client = dest;
    packet.type = SIGNAL;

    // Signal construction
    packet.data.signal.code = code;
    if (buffer != NULL)
        strcpy(packet.data.signal.buffer, buffer);
    return packet;
}
