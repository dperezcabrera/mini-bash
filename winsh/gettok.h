#ifndef __TOKBUF_H
#define __TOKBUF_H

/* Tokens reconocidos */
enum TokType_
{
	EOL,            /* Fin de l�nea */
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
 * a la l�nea de comandos una vez que se ha hecho el parsing.
 */
struct TokBuf
{
        int		length;
        struct Token	* tokens;
};

void comand_exit();

/**
 * userin
 *
 * Retorna el TokBuf resultado de reconocer la cadena entrada por
 * el teclado. Esta funci�n llama a las funciones de userfn.h depeniendo
 * de los caracteres le�dos.
 *
 * @return El Buffer construido
 */
struct Buffer * userin();

/**
 * trataToken
 *
 * A�ade un token nuevo a curTokBuf. curTokBuf guarda el TokBuf actual
 * siendo reconocido a partir de la entrada del usuario. Si hay una
 * cadena temporal en tmpArg, la a�ade antes como un token de tipo ARG.
 *
 * @param type Tipo del token
 * @param string Cadena del token
 */
void trataToken(TokType type, char *string);


/**
 * trataChar
 *
 * A�ade un car�cter al argumento temporal tmpArg.
 *
 * @param c El car�cter.
 */
void trataChar(char c);

/**
 * trataCadCom
 *
 * A�ade una cadena entre comillas simples 
 * al argumento temporal tmpArg.
 *
 * @param s La cadena.
 */
void trataCadCom(char *s);

/**
 * gettok
 *
 * Retorna un TokBuf construido a partir de la cadena dada como par�metro
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

int LeerTecla(HANDLE);
#endif

/* $Id: gettok.h 917 2005-01-17 22:47:03Z dsevilla $ */
