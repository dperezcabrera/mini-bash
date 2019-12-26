#ifndef WSH_C
#define WSH_C

#include "wsh.h"
#include "winsh.h"
#include "string_ext.h"

/**
 * Word_Comand
 *
 * runcomand llama a esta funcion para 
 * ver de que tipo de orden se trata.
 */
static int Word_Comand(char *p)  {
    if ( p ) {
		if ( user_variables_es_definicion(p) )
			return DEFINICION;
        else {
			switch(p[0]) {
				case 'c': if (strcmp(p,  "cd"  ) == 0) return  CD;
					else  if ( strcmp(p,"clear") == 0) return CLEAR; break;
				case 'e':if ( strcmp(p,"echo") == 0) return ECHO;
					else  if (strcmp(p, "env" ) == 0) return  ENV;
					else	 if (strcmp(p,"export") == 0) return  EXPORT;
					else  if (strcmp(p, "exit" ) == 0) return  EXIT; break;
				case 'f': if ((strcmp(p,"false.exe" ) == 0) ||
								 (strcmp(p,"false" ) == 0))return  FALSO; break;
				case 'h': if (strcmp(p, "head" ) == 0) return  HEAD; break;
				case 'l': if (strcmp(p,  "ls"  ) == 0) return  LS;   break;
				case 's': if (strcmp(p, "see"  ) == 0) return SEE;
					else  if (strcmp(p, "set"  ) == 0) return SET;  break;
				case 't': if (strcmp(p, "tail" ) == 0) return  TAIL;
					else  if ((strcmp(p, "true.exe" ) == 0) ||
								(strcmp(p, "true" ) == 0)) return  VERDADERO; break;
				case 'u': if (strcmp(p,"unset" ) == 0) return  UNSET;break;
				default : ;
			}
        }
    }
    return _OTHER_;
}

int Shell_Comand(char *word, int operador){
    int palabra;
	if ( operador & OP_ARROBA )
		return 1;
    if ( operador & OP_PIPE  )
		return 1;
    if ( operador & OP_LOGICO )
		return 1;
    palabra = Word_Comand(word);
    return ( palabra != _OTHER_ );
}

int Shell_Option(char *word){
	int i;
	if ( word ) {
		switch (word[2] ){
			case 'A' : if ( strton(word+3,&i) ) return __A; break;
			case 'a': if ( ( strncmp(word,"--arroba",8) == 0 ) && 
								( strton(word+8,&i) ) )
							return __ARROBA; break;
			case 'c' : if ( strcmp(word,"--comand") == 0 ) return __COMAND; break;
			case 's': if ( strcmp(word,  "--see"  ) == 0 ) return __SEE; break;
			default : return __OTHER;
		}
	}
	return __OTHER;
}

int Shell_Exec (char **comando,int operador,int time){
    int palabra = Word_Comand(comando[0]);
	if ( operador & OP_ARROBA )
		return user_arroba(comando,time);
    if ( operador & OP_PIPE )
		return user_tuberias(comando,operador);
    else if ( operador & OP_LOGICO )
		return user_or_and(comando);
    switch (palabra){
		case CD: return user_cd(comando); break;
		case CLEAR: return user_clear( ); break;
		case DEFINICION: return user_definicion(comando); break;
		case ECHO: return user_echo(comando); break;
		case ENV: return user_env( ); break;
		case EXIT: return user_exit( ); break;
		case EXPORT: return user_export(comando); break;
		case FALSO: return TERMINA_MAL;  break;
		case HEAD: return user_head(comando); break;
		case LS: return user_ls(comando); break;
		case SEE : return user_see(); break;
		case SET : return user_set( ); break;
		case TAIL: return user_tail(comando); break;
		case UNSET : return user_unset(comando); break;
		case VERDADERO: return TERMINA_BIEN; break;
		default : return NOT_FOUND;
    }
}

int Shell_Operators(char **comandos,int num){
	int i = 0;
	int opc = OP_INIT;
	while ( i < num ){
		if ( strcmp(comandos[i],PIPE_S) == 0 )
			opc |= OP_PIPE;
		else if ( strcmp(comandos[i],OR_S) == 0 )
			opc |= OP_LOGICO;
		else if ( strcmp(comandos[i],AND_S) == 0 )
			opc |= OP_LOGICO;
		i++;
	}
	return opc;
}
#endif
