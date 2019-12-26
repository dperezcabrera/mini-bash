#ifndef __TOKBUF_H
#define __TOKBUF_H

#include "buffer.h"

/* Tokens reconocidos */
enum TokType_
{
	EOL,            /* Fin de línea */
	ARG,            /* Argumento */
	AMPERSAND,      /* & */
	SEMICOLON,      /* ; */
	PIPE,           /* | */
	BACKQUOTE,	/* ` */
	QUOTE,          /* ' */
        MENOR,          /* < */
	MAYOR,          /* > */
	MAYORMAYOR,     /* >> */
	DOSMAYOR,       /* 2> */
	DOSMAYORMAYOR,  /* 2>> */

        AND,            /* && */
        OR,             /* || */
        ARROBA,         /* @ */

	SPACE           /* " ", \t, etc. */

};
/* Tipo token */
typedef enum TokType_ TokType;

/* Estructura para almacenar un token junto con su contenido (si procede) */
struct Token
{
	TokType		type;		/* Tipo del token (ver arriba) */
	char		* data;		/* Texto del token */
};

/* Estructura que guarda un conjunto de tokens. Este conjunto representa
 * a la línea de comandos una vez que se ha hecho el parsing.
 */
struct TokBuf
{
	int		length;
	struct Token	* tokens;
};

/**
 * comand_exit
 *
 * Esta funcion modifica una variable global a este modulo y con ello 
 * se consigue que userin devuelva siempre NULL y por tanto se salga
 * del bucle while en smallsh.c.
 * 
 */
void comand_exit();

/**
 * userin
 *
 * Retorna el Buffer resultado de leer la cadena entrada por el teclado
 * Esta función llama a las funciones de userfn.h depeniendo de los
 * caracteres leídos.
 *
 * @return El Buffer construido
 */
struct Buffer * userin();

/**
 * trataToken
 *
 * Añade un token nuevo a curTokBuf. curTokBuf guarda el TokBuf actual
 * siendo reconocido a partir de la entrada del usuario. Si hay una
 * cadena temporal en tmpArg, la añade antes como un token de tipo ARG.
 *
 * @param type Tipo del token
 * @param string Cadena del token
 */
void trataToken(TokType type, char *string);


/**
 * trataChar
 *
 * Añade un carácter al argumento temporal tmpArg.
 *
 * @param c El carácter.
 */
void trataChar(char c);

/**
 * trataCadena
 *
 * Añade una cadena al argumento temporal tmpArg.
 *
 * @param s La cadena.
 */
void trataCadena(char *s);

/**
 * trataCadCom
 *
 * Añade una cadena entre comillas simples 
 * al argumento temporal tmpArg.
 *
 * @param s La cadena.
 */
void trataCadCom(char *s);

/**
 * gettok
 *
 * Retorna un TokBuf construido a partir de la cadena dada como parámetro
 *
 * @param str La cadena
 *
 * @return El TokBuf* con la lista de tokens.
 */
struct TokBuf * gettok(char *str);

/**
 * liberaTokBuf
 *
 * Libera la memoria asociada a un TokBuf
 *
 * @param t El TokBuf.
 */
void liberaTokBuf(struct TokBuf * t);

/**
 * modoInterpretado
 *
 * Establece el modo (interpretado o no) en el descriptor de fichero "fd"
 *
 * @param fd El descriptor de fichero
 * @param on 1=on, 0=off
 */
void modoInterpretado(int fd, int on);

#endif

/* $Id: gettok.h 910 2005-01-11 13:28:59Z dsevilla $ */
