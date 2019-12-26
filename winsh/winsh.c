#include "winsh.h"

int
main(int argc, char **argv)
{
        struct TokBuf * tb;
        struct Buffer * buf;

		if ( argc > 1 )
			return Run_Only_A_Comand(argc,argv);
		else {
			/* Llamar a la funci�n de inicializaci�n del shell */
			user_inicializar();

			/* Procesar �rdenes */
			while ((buf = userin()) != NULL) {
                tb = gettok(buf->data);

                procline(tb);

                /* Liberar el TokBuf que cre� la llamada a userin() */
                liberaTokBuf(tb);
                liberaBuffer(buf);
			}

			/* Finalmente, a la funci�n de finalizaci�n */
			user_finalizar();
		}
        /* Retornar una salida sin error */
        return 0;
}

/* $Id: winsh.c 931 2005-02-03 17:21:07Z dsevilla $ */
