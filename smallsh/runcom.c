#include "smallsh.h"
/*
static jmp_buf primer_plano; 
 
static void sigint_handler(int sig){
  longjmp( primer_plano, 1);
}
*/
/* Ejecuta una orden. "cline" es una array de palabras que contiene el nombre
 * de la orden y los parámetros de dicha orden. "where" indica si dicha
 * orden se debe ejecutar en primer o segundo plano.
 */
int runcommand(char **cline, int where,int operador,unsigned int tiempo) {
        int pid, ret, res;
	int exitstat; 
	int comand_inside = smallsh_comand(cline,operador);
	if ( ( where == FOREGROUND ) && comand_inside )
	   return exec_smallsh(cline,operador);
        if ((pid = fork()) < 0) {
                perror("smallsh");
                return(-1);
        }
	/* Estamos en el padre. Si la orden se ejecuta en segundo plano, no
         * debemos esperar por lo que mostramos el pid del nuevo proceso y
         * regresamos. */
        if ( ( pid != 0 ) && ( where == BACKGROUND ) ) {
            user_nuevo_job(pid,1);
	    return(0);
        }
        if (pid == 0) {			/* Estamos en el hijo. */
	   user_signal_process(where);
	   if ( operador & OP_ARROBA )
	      user_arroba(tiempo,cline[0]);
	   if ( comand_inside ){
	      res = exec_smallsh(cline,operador);
	      exit(res);
	   }
	   else {
	      execvp(*cline, cline);	/* Ejecutamos la orden. */
                                        /* Se llega aquí si no se ha podido
					   ejecutar. Por tanto, se ha producido
					   un error.*/
	      printf("\aERROR: smallsh: %s:",cline[0]);
	      printf(" Comando no encontrado\n");
              exit(127);
	   }
        }

        /* Si la orden se ejecuta en primer plano, debemos esperar a que
	 * termine.*/
        ret = waitpid(pid,&exitstat,WUNTRACED);
	if ( WIFSTOPPED(exitstat) )
	    user_nuevo_job(pid,0);
        return(ret == -1 ? -1 : exitstat);
}

/* $Id: runcom.c 845 2004-10-19 16:08:22Z dsevilla $ */
