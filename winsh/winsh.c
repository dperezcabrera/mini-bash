#include "winsh.h"

int
main(int argc, char **argv)
{
        struct TokBuf * tb;
        struct Buffer * buf;

		if ( argc > 1 )
			return Run_Only_A_Comand(argc,argv);
		else {
			/* Llamar a la función de inicialización del shell */
			user_inicializar();

			/* Procesar órdenes */
			while ((buf = userin()) != NULL) {
                tb = gettok(buf->data);

                procline(tb);

                /* Liberar el TokBuf que creó la llamada a userin() */
                liberaTokBuf(tb);
                liberaBuffer(buf);
			}

			/* Finalmente, a la función de finalización */
			user_finalizar();
		}
        /* Retornar una salida sin error */
        return 0;
}

/* $Id: winsh.c 931 2005-02-03 17:21:07Z dsevilla $ */
