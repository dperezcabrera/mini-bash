#include "winsh.h"
#include "string_ext.h"


int System_Exec(char **comandos, int where,int time){
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
	int i = 0;
	int leng = 1;
	char *cmdline;
	while ( comandos[i]){
	    leng += strlen(comandos[i++]) +1;
	}
	cmdline = (char *)malloc(sizeof(char)*leng);
	i = 1;
	strcpy(cmdline,comandos[0]);
	while ( comandos[i] ){
	    strcat(cmdline," ");
	    strcat(cmdline,comandos[i++]);
	}
	ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );
	  // Creamos el proceso hijo.
	if (!CreateProcess(NULL,      // No nombre de modulo (usar linea de ordenes).
                           cmdline,   // Linea de ordenas a ejecutar.
                           NULL,      // El manejador de proceso no se hereda.
                           NULL,      // El manejador de hilo no se hereda.
                           FALSE,     // Los manejadores no se heredan.
                           0,         // El proceso se crea sin ninguna opcion especial.
                           NULL,      // Heredamos el bloque de entorno del padre.
                           NULL,      // Usamos el directorio de inicio del padre.
                           &si,       // Puntero a estructura STARTUPINFO.
                           &pi)) {     // Puntero a estructura PROCESS_INFORMATION.
            Show_Error_Message(SHELL,comandos[0],"Comando no encontrado");
			return -1;
    }
    /* Si la orden se ejecuta en segundo plano no debemos esperar,
     * por lo que mostramos el pid del nuevo proceso y regresamos. */
     if (where == BACKGROUND) {
         printf("[%d]\n",pi.dwProcessId);
		 CloseHandle(pi.hProcess);
         CloseHandle(pi.hThread);
         return 0;
    }
	if ( time > 0 ) {
		if ( WAIT_TIMEOUT == WaitForSingleObject(pi.hProcess, time*1000) ){
		    printf("\n[Tiempo excedido %d ] %s\n",time,cmdline);
			TerminateProcess(pi.hProcess,-1);
		    CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
		   return -1;
		}
	}
    /* Si la orden se ejecuta en primer plano, debemos esperar a que
      * termine.*/
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return 0;
}

/* Ejecuta una orden. "cline" es una array de palabras que contiene el nombre
 * de la orden y los parámetros de dicha orden. "where" indica si dicha
 * orden se debe ejecutar en primer o segundo plano.
 */

int runcommand(char **cline, int where,int operador,int tiempo) {
    int time = -1;
	if ( operador & OP_ARROBA )
	   time = tiempo;
	if (Shell_Comand(cline[0],operador) )
		return Shell_Exec(cline,operador,tiempo);
	else
		return System_Exec(cline,where,-1);
}

int Run_Only_A_Comand( int number, char ** comand){
	int tiempo;
	int options;
	char **t_word = Word_Options(number-1,comand+1);
	if ( t_word ) {
		if ( t_word[1] )
			Show_Error_Message(SHELL,t_word[1],"Solo se puede Ejecutar una opcion");
		else {
			options = Shell_Option( t_word[0] );
			switch (options ){
				case __SEE: return user_see(); break;
				case __A: strton(t_word[0]+3,&tiempo);
								return user_arroba_exec(number,comand,tiempo); break;
				case __ARROBA: strton(t_word[0]+8,&tiempo);
								return user_arroba_exec(number,comand,tiempo); break;
				case __COMAND:
								return user_comand_exec(number-1,comand+1); break;
				default : {
					Show_Error_Message(SHELL,comand[1],"Opcion invalida");
					return TERMINA_MAL;
				}
			}
		}
	}
	Show_Error_Message(SHELL,"--","Necesita almenos una opcion valida");
	return TERMINA_MAL;
}

/* $Id: runcom.c 848 2004-10-19 16:54:59Z dsevilla $ */
