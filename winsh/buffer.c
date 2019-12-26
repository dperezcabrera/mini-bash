#include "buffer.h"
#include <string.h>
#include <stdlib.h>

/* Tamaños de buffer hasta 1MB */
static int pot2[] = { 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192,
                      16384, 32768, 65536, 131072, 262144, 524288, 1048576 };

struct Buffer *
resize(struct Buffer * b)
{
        int i;

        if ((b->len + 1) < b->max)
                return b;

        i = 0;
        while (pot2[i++] <= (b->len + 1))
                ;

        i--;
        b->max = pot2[i];
        b->data = (char *)realloc(b->data, b->max);

        return b;
}


/* Crea un buffer vacío */
struct Buffer *
creaBuffer(char * buf)
{
        struct Buffer * tmp = (struct Buffer *)malloc( sizeof(struct Buffer) );

        tmp->len = 0;
        tmp->max = pot2[0];
        tmp->data = (char *)malloc(tmp->max);

        tmp->data[0] = 0;
        if (buf != NULL) {
                anyadeCadena(tmp, buf);
        }

        return tmp;
}

void
liberaBuffer(struct Buffer * b)
{
        if (b)
        {
                free(b->data);
                free(b);
        }
}


/* Añade un carácter al final */
struct Buffer *
anyadeChar(struct Buffer * b, char c)
{
        b->len++;

        resize(b);

        b->data[b->len - 1] = c;
        b->data[b->len] = '\0';

        return b;
}

/* Añade una cadena al final */
struct Buffer *
anyadeCadena(struct Buffer * b, char * c)
{
        int len = strlen(c);
        b->len += len;

        resize(b);

        strcat(b->data, c);

        return b;
}



/* Elimina el último carácter del final */
struct Buffer *
eliminaUltimo(struct Buffer * b)
{
        if (b->len > 0)
        {
                b->data[--b->len] = '\0';
        }

        return resize(b);
}

/* $Id: buffer.c 917 2005-01-17 22:47:03Z dsevilla $ */
