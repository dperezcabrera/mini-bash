#ifndef WSH_H
#define WSH_H

#include "userfn.h"

// autores
#define AUTOR1 "David Perez Cabrera"
#define AUTOR2 "Benjamin Valero Espinosa"

// Nombre del shell
#define SHELL "winsh"

// Nombre del fichero de la historia de ordenes
#define SHELL_HISTORY "\\winsh_history"

// Version del shell
#define VERSION "winsh 1.0"

// Fecha de programacion
#define SHELL_FECHA "29/4/2005"

// Variables de este shell
// - directorio home
#define SHELL_HOME "WINSH_HOME"
// - directorio de inicio
#define FST_DIR "WINSH_FIRSTPWD"
// - directorio actual
#define CUR_DIR "WINSH_PWD"
// - directorio anterior
#define OLD_DIR "WINSH_OLDPWD"
// - variable de version
#define NAME_V "WINSH_VERSION"
#define INIT_VAR_SHELL "WINSH_"

// Valor devuelto por las ordenes internas
#define TERMINA_BIEN 0
#define TERMINA_MAL  1
#define NOT_FOUND   -1

// String de los operadores
#define OR_S   "||"
#define AND_S  "&&"
#define PIPE_S  "|"

// Operadores 
#define OP_INIT   0		// Inicial
#define OP_PIPE   1	// Para '|'
#define OP_ARROBA 2 // Para '@n' con n natural
#define OP_LOGICO 4 // Para 'or' ó 'and'

// numero de comandos internos.
#define COMANDOS_INT   12

// Comandos internos de este shell
enum PALABRAS_RESERVADAS_SHELL { 
    FALSO,				// Ejecutable virtual false.exe
	VERDADERO,	// Ejecutable virtual true.exe
	CD,					// Orden cd
	CLEAR,				// Orden clear
	DEFINICION,	// Orden de definir
	ECHO,				// Orden echo
	EXIT,				// Orden exit
	EXPORT,			// Orden export
    ENV,					// Orden env
	HEAD,				// Orden head
	LS,					// Orden ls
	SET,					// Orden set
	SEE,					// Orden see
	TAIL,				// Orden tail
	UNSET,				// Orden unset
	_OTHER_			// Cualquier orden no interna
};

enum OPCIONES_SHELL{
	__SEE,	// Opcion --see
	__A, // Opcion --A'n' con n entero
	__ARROBA, // Opcion --arroba'n' con n entero
	__COMAND, // Opcion --comand
	__OTHER // Cualquier otro --'*' con * cualquier cadena
};
// Funciones exportables
int Shell_Operators(char**,int);
int Shell_Comand(char *,int);
int Shell_Option(char *);
int Shell_Exec (char**,int,int);

#endif
