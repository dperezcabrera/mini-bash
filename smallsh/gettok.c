#include "smallsh.h"

static char * blanco = "                                                                                ";
static int    comando_exit = 0;
static char * especiales = ";&|\\";

/* Mantener contento al compilador */
extern int yy_scan_string();
extern int yylex();

void comand_exit(){
   comando_exit = 1;
}

static char *parteTab(char * d) {
        int i = 0;
        char * q = d;
	char * res = q;
	while ( q[i] ){
	   if ( strchr(especiales,q[i]) != NULL ){
	      i++;
	      res = q + i;
	   }
	   else  if ( q[i] == ' ' ) {
	      while ( q[i] && q[i] == ' ' ) i++;
	      res = q + i;
	   }
	   else if ( q[i] == '\\' ){
	      i++;
	      if ( q[i] == ' ' )
	         while ( q[i] && q[i] == ' ' ) i++;
	      else  if ( q[i] ) i++;
	   }
	   else  if ( q[i] == '\'' ) {
	      i++;
	      while ( q[i] && ( q[i] != '\'' ) ) i++;
	      if ( q[i] == '\'' ) i++;
	   }
	   else 
	      i++;
	}
	return res;
}

static int numeroTab(char * d, int c) {
	int i = 0, res = 1;
        char * q = d;
	while ( q[i] ){
	   if ( strchr(especiales,q[i]) != NULL ){
	      i++,
	      res = 0;
	   }
	   else if ( q[i] == ' ' ) {
	      res++;
	      while ( q[i] && q[i] == ' ' ) i++;
	   }
	   else if ( q[i] == '\\' ){
	      i++;
	      if ( q[i] == ' ' )
	         while ( q[i] && q[i] == ' ' ) i++;
	      else  if ( q[i] ) i++;
	   }
	   else  if ( q[i] == '\'' ) {
	      i++;
	      while ( q[i] && ( q[i] != '\'' ) ) i++;
	      if ( q[i] == '\'' ) i++;
	   }
	   else
	      i++;
	}
	return res;
}

/* "userin" imprime un mensaje de petici�n de �rdenes (prompt) y lee una l�nea.
 *  Dicha l�nea es apuntada por ptr. */
struct Buffer *
userin()
{
        char c;
        int v, tabstatus, count, search, dir;
        char * prompt, * tmpstr;
        struct Buffer * buf;
        struct Buffer * pattern;
	if ( comando_exit )
	   return NULL;
	prompt = user_getPrompt();
        printf("%s ", prompt);
        fflush(stdout);

        buf = creaBuffer(NULL);
        tabstatus = 0;
        count = 0;
        search = 0; /* Estado de b�squeda con las flechas */
        pattern = 0; /* Patr�n de b�squeda */

        while (1 == (v = read(0, &c, 1)))
        {
                /* Flecha arriba: 27 91 65 */
                /* Flecha abajo:  27 91 66 */
                /* Backspace: 127 */
                /* Tabulador: 9 */

                /* Identificar un car�cter normal o una pulsaci�n de
                 * tecla especial
                 */
                switch (c)
                {
                case 27:
                        /* flechas */
                        tabstatus = 0;
                        tmpstr = 0;
                        dir = -1;

                        read(0, &c, 1); /* 91 */
			if (c != 91)
				break;
                        read(0, &c, 1);
			if (c != 65 && c != 66)
				break;
                        switch (c)
                        {
                        case 65: /* arriba */
                                dir = FLECHA_ARRIBA;
                                break;
                        case 66: /* abajo */
                                dir = FLECHA_ABAJO;
                                break;
                        }
                        if (dir != -1)
                        {
                                if (!search)
                                {
                                        /* Si no hay ninguna b�squeda */
                                        search = 1;
                                        pattern = creaBuffer (buf->data);
                                }
                                tmpstr = user_flecha(dir, pattern->data);
                        }

			if (!tmpstr)
				break;
                        liberaBuffer(buf);
                        count = strlen(tmpstr);
                        buf = creaBuffer(tmpstr);
                        free(tmpstr);
		        if ( strcmp(pattern->data,"") ) 
		           printf("\r%s\r%s {%s} %s",
				  blanco, prompt, pattern->data, buf->data);
		        else
		           printf("\r%s\r%s %s",
                               blanco, prompt, buf->data);
                        break;

                case 9:
                        /* TAB */
                        {
                                int i = numeroTab(buf->data, count);
                                tmpstr = parteTab(buf->data);
					
                                if (tabstatus != 2)
                                        tabstatus++;
                                tmpstr = user_tabulador(tmpstr, i, tabstatus);
				if (tmpstr)
                                {
					count += strlen(tmpstr);
					anyadeCadena(buf, tmpstr);
                                        tabstatus = 0;
                                        free(tmpstr);
				}

                        }
                        if (search)
                        {
                                search = 0;
                                liberaBuffer (pattern);
                        }
                        printf("\r%s\r%s %s", blanco, prompt, buf->data);
                        break;

                case 127:
                        /* bs */
                        tabstatus = 0;
                        if (count)
                        {
                                --count;
                                eliminaUltimo(buf);
                                if (search)
                                        printf("\r%s\r%s %s",
                                               blanco, prompt, buf->data);
                                else
                                        printf ("\b \b");
                        }
                        if (search)
                        {
                                search = 0;
                                liberaBuffer (pattern);
                        }
                        break;

                case 10:
                        /* Enter */
                        tabstatus = 0;
                        if (search)
                        {
                                search = 0;
                                liberaBuffer (pattern);
                        }
                        break;

                default:
                        tabstatus = 0;
                        ++count;
                        anyadeChar(buf, c);
                        if (search)
                                printf("\r%s\r%s %s",
                                       blanco, prompt, buf->data);
                        else
                                printf ("%c", c);
                        if (search)
                        {
                                search = 0;
                                liberaBuffer (pattern);
                        }
                }

                fflush(stdout);

                /* Fin al pulsar el intro */
                if (c == 10)
                {
                        printf("\n");
                        break;
                }
        }

        /* Liberar el prompt. Se produce en cada ejecucuci�n de comando */
        free(prompt);

        if (v == -1)
                fprintf(stderr,"Error leyendo de la entrada");

        if (v != 1)
        {
                liberaBuffer(buf);
                return 0;
        }

	/* Informar de una nueva l�nea escrita */
	user_nueva_orden(buf->data);

	return buf;
}


/*
 * Buffer actual de tokens
 */
static struct TokBuf * curTokBuf;

/*
 * Buffer de argumento temporal
 */
static struct Buffer * tmpArg;


/*
 * A�ade un token nuevo a curTokBuf
 */
static void
anyadeToken(TokType type, char *string)
{
        int len;

        len = curTokBuf->length;

        /* Copiar el contenido del buffer en el token actual */
        curTokBuf->tokens =
            realloc(curTokBuf->tokens, (len + 1) * sizeof(struct Token));
        curTokBuf->length++;

        /* Crea el token len-�simo */
        curTokBuf->tokens[len].type = type;
        if (string)
        {
                curTokBuf->tokens[len].data = (char *) malloc(strlen(string) + 1);
                strcpy(curTokBuf->tokens[len].data, string);
        } else {
                curTokBuf->tokens[len].data = 0;
        }
}

void
trataToken(TokType type, char *string)
{
        /* Limpiar el argumento temporal antes de a�adir el token actual. */
        if (tmpArg->len != 0)
        {
                anyadeToken( ARG, tmpArg->data );

                liberaBuffer(tmpArg);
                tmpArg = creaBuffer( NULL );
        }

        /* Ignorar espacios */
        if ( type == SPACE )
                return;

        anyadeToken( type, string );
}

/*
 * A�ade un car�cter al buffer temporal de argumentos tmpArg
 */
void
trataChar(char c)
{
        anyadeChar(tmpArg, c);
}

/*
 * A�ade una cadena al buffer temporal de argumentos tmpArg
 */
void
trataCadena(char *s)
{
    anyadeCadena(tmpArg, s);
}


/*
 * A�ade una cadena al buffer temporal de argumentos tmpArg
 */
void
trataCadCom(char *s)
{
    char *c = strdup(s+1);
    c[strlen(c)-1] = 0;
    anyadeCadena(tmpArg, c);
    free(c);
}

struct TokBuf *
gettok(char *cadena)
{
        /* Inicializar el buffer de tokens a retornar */
        curTokBuf = (struct TokBuf *)malloc(sizeof(struct TokBuf));
        curTokBuf->length = 0;
        curTokBuf->tokens = 0;

        /* Inicializar el argumento temporal a vac�o */
        tmpArg = creaBuffer( NULL );

        yy_scan_string (cadena);
        yylex ();

        /* Insertar un token EOL */
        trataToken(EOL, NULL);

        /* Liberar el buffer temporal */
        liberaBuffer( tmpArg );

        return curTokBuf;
}

void
liberaTokBuf(struct TokBuf * t)
{
        int i;

        for (i = 0; i < t->length; i++)
        {
                free(t->tokens[i].data);
        }

        free(t->tokens);
        free(t);
}


#include <sys/ioctl.h>
#include <termio.h>

void
modoInterpretado(int fd, int on)
{
	static struct termio parametros_ant;
	static int status = 1;
        struct termio parametros;

        if ((on && status) || (!on && !status) || (!isatty(fd)))
		return;
	if (on)
		ioctl(fd, TCSETA, &parametros_ant);
	else {
		// M�rquez, p�g. 167
		ioctl(fd, TCGETA, &parametros_ant);
		parametros = parametros_ant;
		parametros.c_lflag &= ~(ICANON | ECHO);
		parametros.c_cc[4] = 1;
		ioctl(fd, TCSETA, &parametros);
	}
	status = on;
}

/* $Id: gettok.c 944 2005-02-21 18:14:15Z dsevilla $ */
