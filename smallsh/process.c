#ifndef  PROCESOS_C
#define  PROCESOS_C

#define  RUNNING "Ejecutandose"
#define  DONNE   "Terminado   "
#define  STOPPED "Detenido    "

#define  Run    1
#define  Stop   0

#include <wait.h>

struct proceso{
   int  id;
   int  pid;
   int  estado;
   char *orden;
};

typedef struct proceso process;

struct tabla{
   process **tabla_p;
   int numero;
   int maximo;
   int id_ult;
};

typedef struct tabla bbdd_process;

int bbdd_process_init(bbdd_process *bd,int numero){
   int i = 0;
   if ( !bd )
      return 0;
   bd->maximo = numero;
   bd->numero = 0;
   bd->id_ult = 1;
   bd->tabla_p= (process **)malloc(sizeof(process*)*numero);
   while ( i < numero ){
      bd->tabla_p[i] = NULL;
      i++;
   }
   return 1;
}

void bbdd_process_delete(bbdd_process *bd){
   int i = 0;
   if ( bd ){
      while ( i < bd->numero ){
         free(bd->tabla_p[i]);
         i++;
      }
      free(bd->tabla_p);
      bd->tabla_p = NULL;
      bd->numero = 0;
      bd->maximo = 0;
      bd->id_ult = 1;
   }
}

static void redimensionar(bbdd_process *bd){
   bbdd_process baux;
   int i, max = 2*(bd->maximo)+1;
   bbdd_process_init(&baux,max);
   for ( i = 0; i < bd->numero ; i ++)
      baux.tabla_p[i] = bd->tabla_p[i];
   free(bd->tabla_p);
   bd->tabla_p = baux.tabla_p;
   bd->maximo = max;
}

int bbdd_process_show(bbdd_process *bd,int todos){
   int i, j = 0, pid, id, res;
   int result = 0,maximo, status;
   char *comando;
   if ( !bd || (bd->numero == 0))
      return 0;
   maximo = bd->numero;
   for ( i = 0 ; i < maximo ; i++ ){
      pid = bd->tabla_p[i]->pid;
      id  = bd->tabla_p[i]->id;
      comando = bd->tabla_p[i]->orden;
      res = waitpid(pid,&status,WNOHANG|WUNTRACED);
      if (  bd->tabla_p[i]->estado == Stop ){
	 if ( todos ) {
            pid = bd->tabla_p[i]->pid;
	    id  = bd->tabla_p[i]->id;
	    comando = bd->tabla_p[i]->orden; 
	    printf("[%d]  %s\t%s\n",id,STOPPED,comando);
	 }
	 bd->tabla_p[j++] = bd->tabla_p[i];
      }
      else if ( res == 0 ){
	 if ( todos ) {
	    printf("[%d]  %s\t%s\n",id,RUNNING,comando);
	    bd->tabla_p[j++] = bd->tabla_p[i];
	 }
	 bd->tabla_p[j++] = bd->tabla_p[i];
      }
      else {
	 printf("[%d]  %s\t%s\n",id,DONNE,comando);
	 result++;
	 free(bd->tabla_p[i]);
      }
   }
   bd->numero -= result;
   if ( bd->numero != 0 )
      bd->id_ult = (bd->tabla_p[bd->numero-1]->id) + 1;
   else
      bd->id_ult = 1;
   return result;
}

void bbdd_process_insert(bbdd_process *bd,int pid,char *comand,int estado){
   process *p = (process *)malloc(sizeof(process));
   p->id = bd->id_ult++;
   p->pid = pid;
   p->estado = estado;
   p->orden = strdup(comand);
   if ( estado == Run )
      printf("[%d]  %d\n",p->id,p->pid);
   else
      printf("[%d]  %s\t%s\n",p->id,STOPPED,comand);
   if ( bd->maximo == bd->numero )
      redimensionar(bd);
   bd->tabla_p[bd->numero] = p;
   bd->numero++;
}

process *bbdd_process_get_process(bbdd_process *bd,int id){
  int i = 0, pid;
  process *result;
  if ( !bd || (bd->numero == 0))
     return NULL;
  result = bd->tabla_p[0];
  while ( (i < (bd->numero-1)) && (result->id < id) ){
     i++;
     result = bd->tabla_p[i];
  };
  if ( result->id == id ){
     pid = result->pid;
     if ( result->estado == Stop )
        return result;
     else
        return NULL;
  }
  else
     return NULL;
}

#endif
