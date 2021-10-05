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

/* ----------------------------- STD Libraries ----------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* ----------------------------- Definiciones ----------------------------- */
#define TAM_STRING 20

/* --------------------------- Lista de errores --------------------------- */
#define ARG_INCORRECTOS 1
#define ARCHIVO_NO_EXISTE 2
#define ERROR_CIERRE_ARCHIVO 3

/* ------------------------ Prototipos de funciones ------------------------ */

/**
 * @brief Mostrar el uso correcto del ejecutable con sus flags
 * 
 */
void mostrarUso(void);

/**
 * @brief Verificar y manipular los argumentos recibidos
 * 
 * @param argc Numero de argumentos
 * @param argv Vector con los argumentos
 * @param pipeNom RETORNA: nombre del pipe
 * @param fileNom RETORNA: nombre del archivo
 * @return true Se utilizó un archivo
 * @return false No se utilizó un archivo, por lo tanto ignorar el contenido de fileNom
 */
bool manejarArgumentos(int argc, char *argv[], char *pipeNom, char *fileNom);

/**
 * @brief Separar el argumento del nombre de archivo
 * 
 * @param origen Argumento con el nombre de archivo EJ: -pNombrePipe
 * @param destino String que tendrá el nombre del archivo EJ: NombrePipe
 */
void cortarArgumentos(char *origen, char *destino);

/**
 * @brief Intentar abrir un pipeNominal o Archivo en modo de sólo lectura
 * 
 * @param nombre nombre del archivo o pipe
 * @return int File Descriptor del archivo
 */
int abrirArchivo(char *nombre);

/**
 * @brief Cerrar un archivo o un pipe
 * 
 * @param fd File descriptor del archivo a cerrar
 */
void cerrarArchivo(int fd);

/* --------------------------------- Main --------------------------------- */
int main(int argc, char *argv[])
{
    // Manejar los argumentos
    char nombreArchivo[TAM_STRING], nombrePipe[TAM_STRING];
    bool usoArchivo;

    usoArchivo = manejarArgumentos(argc, argv, nombrePipe, nombreArchivo);
    int archivofd = -1, pipefd = -1;

    // Abrir el pipe en sólo lectura
    pipefd = abrirArchivo(nombrePipe);

    // Manejar el archivo
    if (usoArchivo)
    { // Si utilizó el archivo
        // Abrir el archivo
        archivofd = abrirArchivo(nombreArchivo);
    }

    else
    { // Si NO utilizó el archivo
        // Mostrar el menú
    }

    // Cerrar archivos abiertos
    cerrarArchivo(pipefd);
    if (usoArchivo)
        cerrarArchivo(archivofd);

    return EXIT_SUCCESS;
}

/* ----------------------- Definiciones de funciones ----------------------- */
void mostrarUso(void)
{
    fprintf(stderr, "Uso: ./client [-i archivo] -p pipe\n");
    fprintf(stderr, "[-i archivo] es opcional!\n");
    exit(ARG_INCORRECTOS);
}

bool manejarArgumentos(int argc, char *argv[], char *pipeNom, char *fileNom)
{
    // Ya se usaron los argumentos I y P?
    bool argI = false, argP = false, usoArchivo = false;
    char temp1[TAM_STRING], temp2[TAM_STRING];

    // Manipular los argumentos
    switch (argc)
    {
    case 2: // Sólo se usó el argumento -p
        if (argv[1][0] == '-' && argv[1][1] == 'p')
        {
            // Retornar el nombre del pipe
            cortarArgumentos(argv[1], temp1);
            strcpy(pipeNom, temp1);
        }

        else
            mostrarUso();

        break;

    case 3: // Se utilizaron -i y -p

        // Verificar que sólo se hayan utilizado flags -i o -p
        for (int i = 1; i < argc; i++)
            if (argv[i][0] != '-')
                mostrarUso();

        // Filtrar los argumentos
        while ((argc > 1) && (argv[1][0] == '-'))
        {
            switch (argv[1][1])
            {
            case 'i':

                // Verificar si ya se usó el argumento
                if (argI)
                {
                    printf("Argumento ya fue utilizado!: %s\n", argv[1]);
                    mostrarUso();
                }

                // El argumento ya fue utilizado!
                argI = true;
                usoArchivo = true;

                // retornar el nombre de archivo
                cortarArgumentos(argv[1], temp2);
                strcpy(fileNom, temp2);

                break;

            case 'p':

                // Verificar si ya se usó el argumento
                if (argP)
                {
                    printf("Argumento ya fue utilizado!: %s\n", argv[1]);
                    mostrarUso();
                }

                // El argumento ya fue utilizado!
                argP = true;

                // Retornar el nombre del pipe
                cortarArgumentos(argv[1], temp1);
                strcpy(pipeNom, temp1);

                break;

            default:
                printf("Argumento no válido: %s\n", argv[1]);
                mostrarUso();
            }

            argv++;
            argc--;
        }

        break;

    default:
        mostrarUso();
        break;
    }

    return usoArchivo;
}

void cortarArgumentos(char *origen, char *destino)
{
    for (int i = 2, j = 0; i < strlen(origen); i++, j++)
        *(destino + j) = *(origen + i);
}

int abrirArchivo(char *nombre)
{
    // Intentar abrir un pipe en modo de sólo lectura
    int fd = open(nombre, O_RDONLY);
    if (fd < 0)
    {
        perror("Error");
        exit(ARCHIVO_NO_EXISTE);
    }

    return fd;
}

void cerrarArchivo(int fd)
{
    if (close(fd) < 0)
    {
        perror("Error:");
        exit(ERROR_CIERRE_ARCHIVO);
    }
}