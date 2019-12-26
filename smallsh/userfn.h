#ifndef __USERFN_H
#define __USERFN_H

enum
{
	FLECHA_ARRIBA,
	FLECHA_ABAJO
};

/**
 * user_inicializar
 *
 * El shell llama a esta funci�n al principio para que se realicen
 * las acciones de inicializaci�n necesarias.
 */
void user_inicializar(void);

/**
 * user_finalizar
 *
 * El shell llama a esta funci�n al final para que se realicen
 * las acciones de finalizaci�n necesarias.
 *
 * @return entero valor devuelto por la funcion kill().
 */
int user_finalizar(void);

/**
 * user_cd
 *
 * La funcion runcomand en runcom.c llama a esta funcion
 * para que cambie de directorio segun el parametro recibido.
 **/
int user_cd(char **cline);

/**
 * user_getPrompt
 *
 * Devuelve una cadena con el prompt a mostrar. La cadena la debe reservar
 * esta funci�n con malloc.
 *
 * @return cadena de prompt
 */
char * user_getPrompt(void);

/**
 * user_flecha
 *
 * Devuelve la cadena que se debe mostrar en la l�nea de �rdenes al pulsar
 * la flecha arriba o la flecha abajo.
 *
 * @param direccion indica si es flecha arriba o flecha abajo
 * @param patron patr�n a buscar en la historia (anterior o posterior, seg�n
 *               el valor de "direcci�n"
 *
 * @return cadena a poner en la l�nea de �rdenes
 */
char * user_flecha(int direccion, char* patron);

/**
 * user_nueva_orden
 *
 * Esta funci�n es llamada cada vez que el usuario pulsa INTRO, e informa
 * de la orden que ha escrito el usuario. Esta funci�n debe copiar la cadena
 * y no modificar la cadena que se le pasa.
 *
 * @param orden La orden que ha escrito el usuario
 *
 */
void user_nueva_orden(char * orden);

/**
 * user_tabulador
 *
 * Devuelve (si procede) la cadena a a�adir al pulsar el tabulador. La
 * funci�n recibe en "n�mero" si el tabulador se ha pulsado para una orden
 * (un 1) o si se ha pulsado para un argumento (un 2). El argumento "numtab"
 * especifica el n�mero de veces que se ha pulsado el tabulador (1 � 2).
 * En la primera pulsaci�n la funci�n completar� si s�lo hay una opci�n. Si
 * no, emitir� un pitido. En la segunda pulsaci�n, si hay varias posibilidades,
 * se listar�n todas ellas.
 *
 * @param parte parte de la cadena a la que se aplica el tabulador
 * @param numero n�mero del argumento: 1->orden, 2->argumento
 * @param numtab n�mero de veces que se ha pulsado el tabulador (1 � 2)
 *
 * @return Parte restante de la cadena completada o NULL si no se puede
 *         completar
 */
void  user_signal_process(int);
int   exec_smallsh(char**,int);
int   smallsh_comand(char **,int);
char *user_tabulador(char* ,int   ,int);
int   user_comodines(char* ,char**,int);
int   user_tail_head(char**,int);
int   user_definicion(char**);
char *user_variables(char* );
int   user_set(char *);
int   user_unset(char**);
int   user_export(char**);
int   user_ls(char**);
int   user_exit();
void  user_arroba(unsigned int,char*);
int   user_jobs( char **);
int   user_bg  ( char **);
int   user_fg  ( char **);
void  user_nuevo_job(int,int);
void  user_jobs_done();
int   user_or_and  (char **);
int   user_tuberias(char **,int);

#endif

/* $Id: userfn.h 910 2005-01-11 13:28:59Z dsevilla $ */
