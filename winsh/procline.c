#include "winsh.h"
#include "string_ext.h"

#define MAX_RESULTADO 8

/* El siguiente procedimiento procesa una línea de entrada. La ejecución
 * termina cuando se encuentra un final de línea ('\n'). El carácter ';'
 * sirve para separar unas órdenes de otras y el carácter '&' permite
 * una orden en background.
 */
void procline(struct TokBuf * tb) {

	char * arg[MAXARG + 1];	/* Array de palabras: orden + argumentos. */
	TokType toktype;	/* Tipo de token. */
	int narg;		/* Número de argmentos leidos para la orden. */
	int where;		/* ¿ El proceso se ejecutará en primer o segundo
				   plano ?.*/
	unsigned int tiempo;	/* Numero de tiempo cuando se ejecuta un orden
				   que acaba en @'numero' */
    int ntoken;		/* Tokens procesados */
	int    res;		/* Resultado de la ejecucion de un comando */
	char * argumento;	/* Para comodines y variables */
	char    **result;       /* Para actualizar el resultado devuelto */
	char   *auxiliar;
	char    *tok_dat;
	int operador = OP_INIT;
	int salir = 0;
	result    = (char **)malloc(sizeof(char*)*2);
	result[0] = (char  *)malloc(sizeof(char)*MAX_RESULTADO);
	result[1] = NULL;
	narg = 0;
	ntoken = 0;
        while ( ntoken < tb->length && !salir) {
		switch(toktype = tb->tokens[ntoken].type) {
                case ARG:   /* Se ha leido un argumento. */
                        if (narg < MAXARG){
			   tok_dat = tb->tokens[ntoken].data;
			   argumento = strdup(tok_dat);
			   auxiliar = user_variables(argumento);
			   if ( auxiliar != NULL ){
			      free(argumento);
			      argumento = auxiliar;
			   }
			   narg += user_comodines(argumento,arg,narg);
			   if ( ( strcmp(arg[narg-1],AND_S) == 0) ||
			        ( strcmp(arg[narg-1], OR_S) == 0)  )
			      operador |= OP_LOGICO;
			   if ( strcmp(arg[narg-1],PIPE_S) == 0)
			      operador |= OP_PIPE;
		           free(argumento);
			}
                        break;
		case OR : operador  |= OP_LOGICO; 
		          arg[narg++]= strdup( OR_S);
			  break;
		case AND: operador  |= OP_LOGICO;
			  arg[narg++]= strdup(AND_S);
			  break;
		case PIPE:operador  |= OP_PIPE;
		          arg[narg++]= strdup(PIPE_S);
			  break;
		case QUOTE: printf("\aERROR: smallsh:  >'<:");
		            printf(" Se esperaba el fin de comillas simples\n"); 
		            salir = 1; break;
		case ARROBA: operador |= OP_ARROBA;
                case EOL:
                case SEMICOLON:
                case AMPERSAND:
                        where  = (toktype == AMPERSAND) ? BACKGROUND :
                            FOREGROUND;
			if  ( toktype == ARROBA ) {
			   where = BACKGROUND;
			   tiempo = atoi((tb->tokens[ntoken].data)+1);   
			}
			else
			   tiempo = -1;
                        if (narg != 0) {
                                arg[narg] = NULL;
                                res = runcommand(arg,where,operador,tiempo);
				sprintf(result[0],"?=%d",res);
				user_definicion(result);
                        }
                        /* Seguir con la siguiente orden. Esto se da
                         * si se han introducido varias órdenes
                         * separadas por ';' o '&'. */
                        narg   = 0;
                default:
                        ; /* Ignorar */
		}
	 	ntoken++;
        }
	free(result[0]);
	free(result);
}

/* $Id: procline.c 948 2005-02-28 13:21:37Z pedroe $ */
