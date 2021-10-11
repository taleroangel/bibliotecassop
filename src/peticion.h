#ifndef __PETICION_H__
#define __PETICION_H__

#include "libro.h"

struct peticion_t
{
    char peticion;
    char nombre[MAX_NOMLIBRO];
    int isbn;
};

#endif // __PETICION_H__