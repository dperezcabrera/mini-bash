#include "smallsh.h"

int
main(int argc, char **argv)
{
	struct TokBuf * tb;
	struct Buffer * buf;

	/* Llamar a la función de inicialización del shell */
	user_inicializar();

	/* Establecer modo no interpretado en la entrada */
	modoInterpretado(0, 0);

	/* Procesar órdenes */
	while ((buf = userin()) != NULL)
	{
	        tb = gettok(buf->data);

		procline(tb);

		/* Liberar el TokBuf que creó la llamada a userin() */
		liberaTokBuf(tb);
		liberaBuffer(buf);
	}

	/* Restaurar el modo de la entrada */
	modoInterpretado(0, 1);

	/* Finalmente, a la función de finalización */
	user_finalizar();

	/* Retornar una salida sin error */
	return 0;
}

/* $Id: smallsh.c 845 2004-10-19 16:08:22Z dsevilla $ */
