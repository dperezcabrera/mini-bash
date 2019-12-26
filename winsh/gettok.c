#include "winsh.h"

static char blanco[] = "                                                                               ";

static int    comando_exit = 0;
static char * especiales = ";&|";

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
	   else if ( q[i] == '-' ){
	      i++;
	      if ( q[i] == ' ' )
	         while ( q[i] && q[i] == ' ' ) i++;
	      else  if ( q[i] ) i++;
	   }
	   else  if ( q[i] == '"' ) {
	      i++;
	      while ( q[i] && ( q[i] != '"' ) ) i++;
	      if ( q[i] == '"' ) i++;
	   }
	   else 
	      i++;
	}
	return res;
}

static int numeroTab(char * d) {
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
	   else if ( q[i] == '-' ){
	      i++;
	      if ( q[i] == ' ' )
	         while ( q[i] && q[i] == ' ' ) i++;
	      else  if ( q[i] ) i++;
	   }
	   else  if ( q[i] == '"' ) {
	      i++;
	      while ( q[i] && ( q[i] != '"' ) ) i++;
	      if ( q[i] == '"' ) i++;
	   }
	   else
	      i++;
	}
	return res;
}

/* "LeerTecla" lee la siguiente pulsacion de tecla, devolviendo
 * el codigo virtual de la tecla pulsada (para teclas con un tratamiento
 * especial, como tabulador, retroceso, etc.) o el caracter que representa.
 */
int LeerTecla(HANDLE entrada)
{
        DWORD leidos = 0;
        INPUT_RECORD InBuf;
        int c;
        MSG msg = { 0, 0, 0, 0 };

        /* Esperar a que o bien haya una tecla en la entrada o un mensaje a la
           aplicación */
        while (1)
        {
                /* Comprobar si hay algún mensaje en la cola y procesarlo */
                if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                {
                        /* Hacer el dispatch sólo de WM_TIMER. Esto hará
                           que se llame la función del timer del usuario */
                        DispatchMessage(&msg);
                }

                /* Evitar la espera activa. Con 10ms es suficiente */
                Sleep (10);

                GetNumberOfConsoleInputEvents (entrada, &leidos);

                if (leidos >= 1)
                {
                        if (!ReadConsoleInput(entrada, &InBuf, 1, &leidos) ||
                            (leidos != 1))
                        {
                                perror("ReadConsoleInput");
                                return VK_RETURN;
                        }

                        /* Si se ha pulsado una tecla...*/
                        if (InBuf.EventType == KEY_EVENT &&
                            InBuf.Event.KeyEvent.bKeyDown)
                        {
                                switch (InBuf.Event.KeyEvent.wVirtualKeyCode)
                                {
                                case VK_TAB:
                                case VK_UP:
                                case VK_DOWN:
                                case VK_BACK:
                                case VK_RETURN:
                                        /* El desplazamiento a la izquierda lo necesitamos porque
                                         * VK_UP=38 y VK_DOWN=40. El primero coincide con el
                                         * carácter '&' y el segundo con el carácter '('. */
                                        return InBuf.Event.KeyEvent.wVirtualKeyCode << 8;
                                default:
                                        c = InBuf.Event.KeyEvent.uChar.AsciiChar;
                                        if (c >= 32)
                                                return c;
                                }
                        }
                }
        }
}

/* "userin" imprime un mensaje de petición de órdenes (prompt) y lee una línea.
 *  Dicha línea es apuntada por ptr. */
struct Buffer *
userin(void)
{
        int c, tabstatus, count, search, dir;
        char * prompt, * tmpstr;
        struct Buffer * buf;
        struct Buffer * pattern;
        HANDLE entrada;
		
		if ( comando_exit ) return NULL;
        prompt = user_getPrompt();
        printf("%s ", prompt);
        fflush(stdout);

        buf = creaBuffer(NULL);
        tabstatus = 0;
        count = 0;
        search = 0; /* Estado de búsqueda con las flechas */
        pattern = 0; /* Patrón de búsqueda */

        if (INVALID_HANDLE_VALUE == (entrada = GetStdHandle(STD_INPUT_HANDLE)))
        {
                return NULL;
        }

        while ((VK_RETURN << 8 ) != (c = LeerTecla(entrada)))
        {
                /* Ver "LeerTecla" para la explicación de "<< 8" */
                /* Flecha arriba: VK_UP << 8*/
                /* Flecha abajo:  VK_DOWN << 8*/
                /* Backspace: VK_BACK << 8*/
                /* Tabulador: VK_TAB << 8*/
                /* Return: VK_RETURN << 8*/

                /* Identificar un carácter normal o una pulsación de
                 * tecla especial
                 */
                switch (c)
                {
                case VK_UP << 8:
                case VK_DOWN << 8:
                        /* flechas */
                        tabstatus = 0;
                        tmpstr = 0;
                        dir = -1;

                        switch (c) {
                                case VK_UP << 8: /* arriba */
                                        dir = FLECHA_ARRIBA;
                                        break;
                                case VK_DOWN << 8: /* abajo */
                                        dir = FLECHA_ABAJO;
                                        break;
                        }
                        if (dir != -1)
                        {
                                if (!search)
                                {
                                        /* Si no hay ninguna búsqueda */
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
						if ( pattern->data[0] )
                            printf("\r%s\r%s {%s} %s",
                            blanco, prompt, pattern->data, buf->data);
						else
						    printf("\r%s\r%s %s",blanco, prompt, buf->data);
                        break;

                case VK_TAB << 8:
                        /* TAB */
                        {
                                int i = numeroTab(buf->data);
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

                case VK_BACK << 8:
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
                                        printf("\b \b");
                        }
                        if (search)
                        {
                                search = 0;
                                liberaBuffer (pattern);
                        }
                        break;

                case VK_RETURN << 8:
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
                        anyadeChar(buf, (char) c);
                        if (search)
                                printf("\r%s\r%s %s",
                                       blanco, prompt, buf->data);
                        else
                                printf("%c", (char) c);
                        if (search)
                        {
                                search = 0;
                                liberaBuffer (pattern);
                        }
                }

                fflush(stdout);
        }

        printf("\n");
        fflush(stdout);

        /* Liberar el prompt. Se produce en cada ejecucución de comando */
        free(prompt);

        /* Informar de una nueva línea escrita */
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
 * Añade un token nuevo a curTokBuf
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

        /* Crea el token len-ésimo */
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
        /* Limpiar el argumento temporal antes de añadir el token actual. */
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
 * Añade un carácter al buffer temporal de argumentos tmpArg
 */
void
trataChar(char c)
{
        anyadeChar(tmpArg, c);
}

/*
 * Añade una cadena al buffer temporal de argumentos tmpArg
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

        /* Inicializar el argumento temporal a vacío */
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

/* $Id: gettok.c 959 2005-03-29 21:23:39Z dsevilla $ */
