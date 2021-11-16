#include "../buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void mostrarlibro(book_t libro)
{
    printf("NOMBRE: %s\nISBN: %d\nCOPIAS: %d\n", libro.name, libro.ISBN, libro.n_copies);
}

int main()
{
    buffer_t cola;
    cola.n_items = 0;
    cola.paquetArray = (paquet_t *)malloc(sizeof(paquet_t) * cola.n_items);

    // Libro 1
    book_t libro;
    libro.petition = SOLICITAR;
    strcpy(libro.name, "Prueba1");
    libro.ISBN = 001;
    libro.n_copies = 1;

    paquet_t paquete;
    paquete.client = 1;
    paquete.type = BOOK;
    paquete.data.libro = libro;

    // Libro 2
    book_t libro2;
    libro2.petition = SOLICITAR;
    strcpy(libro2.name, "Prueba2");
    libro2.ISBN = 002;
    libro2.n_copies = 2;

    paquet_t paquete2;
    paquete2.client = 2;
    paquete2.type = BOOK;
    paquete2.data.libro = libro2;

    // Añadir al buffer
    queue(&cola, paquete);
    queue(&cola, paquete2);

    // Mostrar libros
    printf("Libro1\n");
    mostrarlibro(getLast(&cola)->data.libro);
    dequeue(&cola);

    printf("res: %d", getLast(&cola));

    return 0;
}