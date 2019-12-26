#include "list.h"
#include "smallsh.h"
#include "string_ext.h"
#include "userfn.h"
#include "process.c"

#define MAX_HISTORIA 1000 // maximo de  ordenes guardadas  en la  historia.
#define MAX_LINEA    4096 // maximo de  linea que  puede tener la historia.
#define MAX_OPCIONES  100 // maximo de opciones distintas que pueden tener.
#define COMANDOS_INT   11 // numero de comandos internos.

extern char **environ;
extern int errno ;

//////////////////////////////////////////////////////////////////////////////////////
//  Definicion variables globales que se necesitaran para las caracteristicas.      //
//////////////////////////////////////////////////////////////////////////////////////

list_t variables ;
list_t  historia ;
list_it posicion ;
bbdd_process bdP ;

static int long_columns =  80;
static jmp_buf funcion_arroba;
static char     *ultima_orden;

//////////////////////////////////////////////////////////////////////////////////////
//  Definicion variables globales que no seran modificadas. ( las constantes )      //
//////////////////////////////////////////////////////////////////////////////////////

static char  permisos_c[]    = { 'x', 'w', 'r' };
static char  mes_cadena[][4] = { "Ene", "Feb", "Mar", "Abr", "May", "Jun", 
                                "Jul", "Ago", "Sep", "Oct", "Nov", "Dic"};
static char  comandos_internos[][7] = 
                     { "bg", "cd", "exit", "export", "fg", "head",
		       "jobs" , "ls" , "set" , "tail" , "unset "};

//////////////////////////////////////////////////////////////////////////////////////
//  Definicion de funciones que se necesita saber que estaran implementadas.        //
//////////////////////////////////////////////////////////////////////////////////////

static void user_signal_init( );
static void aux_historia_cargar ( );
static void aux_historia_guardar( );
static int  aux_ls_directorio(char *,int,int);
static int  aux_ls_ficheros  (char**,int,int);
static int  aux_ls_errores   (char *);
static int  aux_variables_es_definicion(char *);


//////////////////////////////////////////////////////////////////////////////////////
// ( A ) //////////////    INICIO DE  FUNCIONES AUXILIARES    ////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//										    //
//  Funciones:									    //
//		int comparador_con_mayusculas(...);				    //
//		int comparador_ultima_palabra(...);				    //
//		char **ordenar_en_columnas(...);				    //
//		int    opciones_con_menos (...);				    //
//		int    palabra_reservada  (...);				    //
//										    //
//     Estas funciones  estan implementadas  para simplificar  las funciones	    //
//  de cada  caracteristica, en ella se recogen  algoritmos comunes, como el	    //
//  que permite ordenar la salida en columnas. Otro como  el de opciones con	    //
//  menos, solo su utiliza  para la  caracteristica ls, pero como es general	    //
//  y no tiene ninguna dependencia de las funciones  de la caracteristica ls	    //
//  pues es una funcion  auxiliar, de este modo  si se desea tener en cuenta	    //
//  las opciones de cualquier otra caracteristica se puede hacer uso de esta	    //
//  esta funcion.								    //
//										    //
//////////////////////////////////////////////////////////////////////////////////////


static int comparador_con_mayusculas(const char*c1, const char *c2){
	char *aux1, *aux2;
	int res ;
	if ( !c1 || !c2 ){
	   perror("Comparacion con un NULL");
	   return 0;
	}
	aux1 = strtup(strdup(c1));
	aux2 = strtup(strdup(c2));
	res  = strcmp(aux1, aux2);
	free(aux1);
	free(aux2);
	return res;
}

static int comparador_ultima_palabra(const char *c1, const char *c2){
	char *aux1, *aux2;
	if ( !c1 || !c2 ){
	   perror("Comparacion con un NULL");
	   return 0;
	}
	aux1 = strrchr(c1,' ');
	aux2 = strrchr(c2,' ');
	return comparador_con_mayusculas(aux1,aux2);
}

// Funcion que pasa una tabla de string  a otra tabla
// ordenada por columna
static char** ordenar_en_columnas(char **t,int size,cmp c){
   char **result, *fich;
   int longitud_mayor = 0, columnas, filas, lenght, j, i = 0;
   while ( i < size ){
      lenght = strlen( t[i] );
      if ( longitud_mayor < lenght )
	 longitud_mayor = lenght;
      i++;
   }
   strQuickSort(t,size,c);
   columnas = long_columns / (longitud_mayor + 1) ;
   if ( columnas != 0 ) {
      filas = size / columnas;
      if ( size % columnas )
         filas++;
   }
   if ( columnas > 1 ){
      result = (char **)malloc(sizeof(char*)*(filas+1));
      for ( j = 0 ; j < filas ; j++){
         result[j] = (char *)malloc(sizeof(char)*(long_columns+1));
	 strcpy(result[j],"");
	 for ( i = 0; i < columnas ; i++){
	    if (( j + i*filas ) < size ){
	       fich = strers(t[i*filas + j],(longitud_mayor+1),' ');
	       strcat(result[j],fich);
	       free(fich);
	    }
	 }
      }
      result[filas] = NULL;
   }
   else {
      result = (char **)malloc(sizeof(char*)*(size+1));
      for( i = 0; i < size ; i++ )
         result[i] = strdup(t[i]);
      result[size] = NULL;
   }
   return result;
}

// Funcion que devuelve todos las opciones de una letra
// que comienzan por -;
static int opciones_con_menos(char **comando,char *opc){
	int i = 0, j;	                               
	int res = 0;
	list_t lista;
	list_new(&lista);
	char op[2];
	op[1] = 0;
	op[0] = 0;
	while ( comando[i] ){
	   if ( comando[i][0] == '-'){
	      if ( ( strlen(comando[i]) > 1) &&
	           ( comando[i][1] != '-'  ) ) {
	         j = 1;
	      	 while (comando[i][j]){
	      	    op[0] = comando[i][j];
		    list_erase(&lista,op);
		    list_insert(&lista,op);
		    j++;
	         }
	      }
	   }
	   else
	      res++;
	   i++;
	}
	i = 0;
	if ( op[0] ){
	   while ( list_size(&lista)> 0 ){
	      opc[i] = (list_front(&lista))[0];
	      list_pop_front(&lista);
	      i++;
	   }
	}
	opc[i] = 0;
	return res;
}

/**
 * palabra reservada
 *
 * runcomand llama a esta funcion para 
 * ver de que tipo de orden se trata.
 */
static int palabra_reservada(char *p)  {
   
   int res = NINGUNA ;
   if ( p ) {
      if ( aux_variables_es_definicion(p) )
	res = DEFINICION ;
      else {
	 switch(p[0]) {
	  case 'b':if (strcmp(p,  "bg"  ) == 0) res = BG;   break;
	  case 'c':if (strcmp(p,  "cd"  ) == 0) res = CD;   break;
	  case 'e':if (strcmp(p,"export") == 0) res = EXPORT;
	      else if (strcmp(p, "exit" ) == 0) res = EXIT; break;
	  case 'f':if (strcmp(p,"false" ) == 0) res = FALSO;
	      else if (strcmp(p,  "fg"  ) == 0) res = FG;   break;
	  case 'h':if (strcmp(p, "head" ) == 0) res = HEAD; break;
	  case 'j':if (strcmp(p, "jobs" ) == 0) res = JOBS; break;
	  case 'l':if (strcmp(p,  "ls"  ) == 0) res = LS;   break;
	  case 's':if (strcmp(p, "set"  ) == 0) res = SET;  break;
	  case 't':if (strcmp(p, "tail" ) == 0) res = TAIL;
	      else if (strcmp(p, "true" ) == 0) res = VERDADERO; break;  
	  case 'u':if (strcmp(p,"unset" ) == 0) res = UNSET;break;
	  default :{;}	     
	 }
      }   
   }
   return res;
}

//////////////////////////////////////////////////////////////////////////////////////
// ( A ) ////////////////    FIN DE FUNCIONES AUXILIARES    //////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// ( E ) ///////////    INICIO DE OTRAS FUNCIONES EXPORTABLES    /////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//										    //
//  La funcion : int palabra_reservada(...);					    //
//										    //
//  Es la unica funcion exportable que no pertenece a ninguna caracteristica.	    //
//										    //
//////////////////////////////////////////////////////////////////////////////////////


int smallsh_comand(char **cline, int operador){
   int palabra;
   if ( operador & OP_PIPE  )
      return 1;
   if ( operador & OP_LOGICO )
      return 1;
    palabra = palabra_reservada(cline[0]);
    return ( palabra != NINGUNA );
}

int exec_smallsh(char **cline,int operador){
      int palabra = palabra_reservada(cline[0]);
      if ( operador & OP_PIPE )
	 return user_tuberias(cline,operador);
      else if ( operador & OP_LOGICO )
	 return user_or_and(cline);
      switch (palabra){
      	 case BG:         return user_bg(cline); break;
	 case FALSO:      return TERMINA_MAL;  break;
	 case VERDADERO:  return TERMINA_BIEN; break;
         case EXIT:       return user_exit( ); break;
	 case LS:         return user_ls(cline); break;
	 case CD:         return user_cd(cline); break;
	 case FG:         return user_fg(cline); break;	 	   
	 case SET :       return user_set(cline[1]); break;
	 case JOBS:       return user_jobs ( cline); break;
	 case UNSET :     return user_unset( cline); break;
	 case EXPORT:     return user_export(cline); break;
	 case DEFINICION: return user_definicion( cline); break;
	 case HEAD:       return user_tail_head(cline,0); break;
	 case TAIL:       return user_tail_head(cline,1); break;
	 default : return NOT_FOUND;
      }
}

//////////////////////////////////////////////////////////////////////////////////////
// ( E ) /////////////    FIN DE OTRAS FUNCIONES EXPORTABLES    //////////////////////
//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// ( G ) ///////////////    INICIO DE FUNCIONES GENERALES    /////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

void user_inicializar(void) {

	user_signal_init   (  );
	aux_historia_cargar(  );
	list_new ( &variables );
	ultima_orden=strdup("");
	bbdd_process_init(&bdP,10);
}

int user_finalizar(void){

   	aux_historia_guardar(   );
        list_delete( &variables );
   	free(ultima_orden);
	printf( "Exit\n" );
   	return 1;
}

//////////////////////////////////////////////////////////////////////////////////////
// ( G ) ////////////////    FIN DE FUNCIONES  GENERALES    //////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// [ 1 ] ///////////    INICIO DE FUNCIONES PARA LAS SENYALES    /////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//										    //
//  Funciones implementadas:							    //
//			     void user_signal_init   (...); 			    //
//  			     int  user_signal_process(...);			    //
//										    //
//////////////////////////////////////////////////////////////////////////////////////

static void user_signal_init(){
  	signal(SIGINT ,SIG_IGN);
  	signal(SIGQUIT,SIG_IGN);
  	signal(SIGTERM,SIG_IGN);
  	signal(SIGTSTP,SIG_IGN);
}

void user_signal_process(int donde){
  // todos los hijos deben escuchar SIGTERM
  signal(SIGTERM,SIG_DFL);
  if ( donde == BACKGROUND ) {
     signal(SIGINT ,SIG_IGN);
     signal(SIGQUIT,SIG_IGN);
     signal(SIGTSTP,SIG_IGN);
  }
  // ( donde == FOREGROUND )
  else {
     signal(SIGINT ,SIG_DFL);
     signal(SIGQUIT,SIG_DFL);
     signal(SIGTSTP,SIG_DFL);    
  }
}

//////////////////////////////////////////////////////////////////////////////////////
// [ 1 ] ////////////    FIN DE FUNCIONES  PARA LAS SENYALES    //////////////////////
//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// [ 2 ] ///////////    INICIO DE LA FUNCION PARA LA ORDEN CD    /////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//										    //
//  Esta caracteristica solo necesita la funcion: int user_cd(...);		    //
//										    //
//////////////////////////////////////////////////////////////////////////////////////


int user_cd(char **cline) {
	/* Implementación */   
   int result = TERMINA_BIEN;
   char *anterior;
   char ruta[MAX_LINEA];
   anterior = strdup(getenv("PWD"));
   // cambio al directorio base
   if (cline[1] == NULL) {
      chdir(getenv("HOME")) ;
   }
   // cambio al directorio anterior
   else if (!strcmp(cline[1], "-" )) {
      if ( getenv("OLDPWD") ){
	chdir(getenv("OLDPWD"));
	printf("%s\n",getenv("OLDPWD"));
      }
      else {
	printf("\aERROR: smallsh: cd: No hay un directorio anterior\n");
        result = TERMINA_MAL;
      }
   }
   else {
	if ( chdir(cline[1]) )
	   printf("\aERROR: smallsh: %s: No es un directorio\n",cline[1]);
   }
   getcwd(ruta,MAX_LINEA-1);
   setenv("PWD",(const char *)ruta,1);
   setenv("OLDPWD",(const char *)anterior,1);
   return result;
}

//////////////////////////////////////////////////////////////////////////////////////
// [ 2 ] ////////////    FIN DE LA FUNCION  PARA LA ORDEN CD    //////////////////////
//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// [ 3 ] ////////    INICIO DE LA FUNCION PARA OBTENER EL PROMPT    //////////////////
//////////////////////////////////////////////////////////////////////////////////////
//										    //
//  Esta caracteristica solo necesita la funcion: char *user_getPrompt(...);	    //
//										    //
//////////////////////////////////////////////////////////////////////////////////////


char * user_getPrompt(void){
	/* Implementación */
  char *prompt;
  char *nombre = getlogin();
  char *ruta = strdup(getenv("PWD"));
  char *directorio = strrchr(ruta,'/');
  char c = '$';
  int promptLeng = 20;
  struct tm *tp;
  time_t t;
  time(&t);
  tp = localtime(&t);
  promptLeng += ( strlen(nombre) + strlen(directorio) );
  if ( !strcmp("root",nombre) ) { c = '#'; }
  prompt = (char *) malloc ( sizeof(char) * promptLeng );
  sprintf(prompt,"[%d%d/%d%d/%d%d %d%d:%d%d %s@%s]%c",
	( tp->tm_mday/10),    ( tp->tm_mday  % 10),
	((tp->tm_mon + 0)/10),((tp->tm_mon+1)% 10),
	((tp->tm_year/10)%10),( tp->tm_year  % 10),
	( tp->tm_hour/10),    ( tp->tm_hour  % 10),
	( tp->tm_min /10),    ( tp->tm_min   % 10),
	( nombre ),           (directorio+1), (c));
  free(ruta);
  return prompt;
}

//////////////////////////////////////////////////////////////////////////////////////
// [ 3 ] /////////    FIN DE LA FUNCION  PARA OBTENER EL PROMPT    ///////////////////
//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// [ 4 ] ///////    INICIO DE FUNCIONES DEL MECANISMO DE TUBERIAS    /////////////////
//////////////////////////////////////////////////////////////////////////////////////

int aux_tuberias_sintaxis(char **comandos){
   int i = 1;
   if  ( strcmp(comandos[0],PIPE_S) == 0) {
	printf("\aERROR: smallsh: |:");
	printf(" Se esperaba un token inicial\n");
	return 0;
   }
   while (comandos[i]){
      if ( ( ( strcmp(comandos[i],PIPE_S) == 0) ) &&
	   ( strcmp(comandos[i-1],PIPE_S) == 0) ) {
	  printf("\aERROR: smallsh: |:");
	  printf(" Se esperaba un token despues del operador\n");
	  return 0;     
      }
      i++;
   }
   if ( strcmp(comandos[i-1],PIPE_S) == 0) {
      printf("\aERROR: smallsh: %s:",(1+comandos[i-1]));
      printf(" Se esperaba un token final\n");
      return 0;
   }
   return 1;
}

int user_tuberias(char **comandos,int opciones){
   int  j, i, pid, res;
   int  resultado;
   int  es_primero = 1;
   int  opcion = (opciones & (~OP_PIPE));
   int  tuberias[2][2];
   int  pipe_ENTRADA = 1;
   int  pipe_SALIDA = 0;
   char *comand[MAXARG];
   i = 0;
   if ( !aux_tuberias_sintaxis(comandos) )
      return TERMINA_MAL;
   if ( (pid = fork()) == 0 ){
      while ( comandos[i] ){
         j = 0;
         while ( comandos[i]  &&
                (strcmp(comandos[i],PIPE_S)!= 0)){
	    comand[j++] = comandos[i++];
         }
	 comand[j++] = NULL;
	 // si no es el ultimo
         if ( comandos[i] ){
	    pipe_ENTRADA = !pipe_ENTRADA;
	    pipe_SALIDA  = !pipe_SALIDA;
            comandos[i++] = NULL;
            if ( pipe(tuberias[pipe_ENTRADA]) < 0 ){
	       perror("pipe");
	       exit(-1);
	    }
	    if ( fork() == 0 ) {
	       if ( !es_primero ) {
	          close(0);
		  dup  (tuberias[pipe_SALIDA][0]);
	          close(tuberias[pipe_SALIDA][1]);
		  close(tuberias[pipe_SALIDA][0]);
	       }
	       close(1);
               dup  (tuberias[pipe_ENTRADA][1]);
	       close(tuberias[pipe_ENTRADA][1]);
	       close(tuberias[pipe_ENTRADA][0]);
	       if ( smallsh_comand(comand,opcion) ){
	          res = exec_smallsh(comand,opcion);
		  exit(res);
	       }
	       else{
	          execvp( comand[0], comand);
		  printf("\aERROR: smallsh: %s:",comand[0]);
	          printf(" Comando no encontrado\n");
		  wait(NULL);
		  exit(-1);
	       }   
	    }
	    else {
	       es_primero = 0;
	       close(tuberias[pipe_SALIDA][0]);
	       close(tuberias[pipe_SALIDA][1]);
	    }
	   
         }
      }
      close(0);
      dup  (tuberias[pipe_ENTRADA][0]);
      close(tuberias[pipe_ENTRADA][0]);
      close(tuberias[pipe_ENTRADA][1]);
      if ( fork() == 0 ) {
         if ( smallsh_comand(comand,opcion) ){
	    res = exec_smallsh(comand,opcion); 
	    exit(res);
         }
         else {
	    execvp( comand[0], comand);
            printf("\aERROR: smallsh: %s:",comand[0]);
            printf(" Comando no encontrado\n");
	    exit(-1);
         }
       }
       else {
	  while ( wait(NULL) != -1 );
	  return 0;
       }
   }
   else {
     waitpid(pid,&resultado,0);
     return (resultado == 0) ? TERMINA_BIEN : TERMINA_MAL;
   }
}

//////////////////////////////////////////////////////////////////////////////////////
// [ 4 ] ////////    FIN DE FUNCIONES DEL  MECANISMO DE TUBERIAS    //////////////////
//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// [ 5 ] //////    INICIO DE FUNCION PARA EL SOPORTE DE COMODINES    /////////////////
//////////////////////////////////////////////////////////////////////////////////////
//										    //
//  Esta caracteristica solo necesita la funcion: int user_comodin(...);	    //
//										    //
//////////////////////////////////////////////////////////////////////////////////////


int user_comodines(char *patron,char **destino,int i){
	glob_t g;
	int j = 0;
	if ( strnss(patron,"[]{}*?") != NULL && !strsub(patron,"$?")){
	   if ( glob(patron,GLOB_BRACE,NULL,&g) != GLOB_NOMATCH ){
	      while ( i < MAXARG && g.gl_pathv[j] != NULL ){
		 destino[i++] = strdup(g.gl_pathv[j]);
		 j++;
	      }
	      globfree(&g);
	      return j;
	   }
	}
	destino[i++] = strdup(patron);
	return 1;
}

//////////////////////////////////////////////////////////////////////////////////////
// [ 5 ] ////////    FIN DE FUNCION PARA EL SOPORTE DE COMODINES    //////////////////
//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// [ 6 ] ///////    INICIO DE FUNCIONES DE LA HISTORIA DE ORDENES    /////////////////
//////////////////////////////////////////////////////////////////////////////////////
//										    //
//  Esquema general de la implementacion de la historia de ordenes:		    //
//										    //
//  Funciones:									    //
//										    //
//  Principal:									    //
//               char *user_flecha(...);					    //
//		 void  user_nueva_orden(...);					    //
//  Secundarias:								    //
//		 void aux_historia_cargar (...);				    //
//		 void aux_historia_guardar(...);				    //
//										    //
//										    //
//////////////////////////////////////////////////////////////////////////////////////


static void aux_historia_cargar(){

  int ind_linea = 0;
  char linea[MAX_LINEA] ;
  FILE *f_small_h;
  char c = 0;
  int long_linea;
  char *ruta;
  char *orden_vacia = (char *)malloc(sizeof(char)) ;  

  orden_vacia[0] = '\0';
  list_new(&historia) ;
  long_linea = strlen(getenv("HOME"))+18 ;
  ruta = (char *)malloc(sizeof(char)*long_linea) ;
  sprintf( ruta,"%s/.smallsh_history",getenv("HOME")) ;

  if ((f_small_h = fopen(ruta,"r")) != NULL) {
    fscanf(f_small_h,"%c",&c) ;
    while( !feof(f_small_h) ) {
      ind_linea = 0 ;    
      while( !feof(f_small_h) && (c != '\n') && (ind_linea < MAX_LINEA) ) {
	linea[ind_linea] = c ;
	fscanf(f_small_h,"%c",&c) ;
	ind_linea++ ;
      }
      if ( ind_linea > 0 ) {  
	linea[ind_linea]='\0';
	list_push_back(&historia,linea) ;
      }
      fscanf(f_small_h,"%c",&c) ;
    }
    fclose(f_small_h) ;
  }
  list_push_back(&historia,orden_vacia) ;
  list_it_copy(&posicion,list_last(&historia) ) ;
  free(ruta);
}

static void aux_historia_guardar(){

   FILE *f;
   char *ruta;
   int long_linea;	
   long_linea = strlen(getenv("HOME"))+18 ;
   ruta = (char *)malloc(sizeof(char)*long_linea) ;
   sprintf( ruta,"%s/.smallsh_history",getenv("HOME")) ;
   f = fopen(ruta,"w+");
   while ( list_size(&historia) > 1 ){
      fprintf(f,"%s\n",list_front(&historia));
      list_pop_front(&historia);
   }
   fclose(f);
   list_delete(&historia);
}

char *user_flecha(int direccion, char* patron){
	/* Implementacion*/
  char *orden = NULL,*aux;
  list_it posic;
  list_it_copy(&posic,&posicion);
  if ( direccion == FLECHA_ARRIBA ) {
    if ( list_it_is_first(&posic) )
      printf("\a");
    else {
        list_it_previous(&posic);
      	aux = strdup(list_it_element(&posic));
	while ( !list_it_is_first(&posic) && !strsub(aux,patron)){
	  list_it_previous(&posic);
	  free(aux);
      	  aux = strdup(list_it_element(&posic));
	}
	free(aux);
	aux = strdup(list_it_element(&posic));
	if (strsub(aux,patron))
	   orden = strdup(list_it_element(&posic));
	else {
	  printf("\a");
	  free(aux);
	  return NULL;
	}
	free(aux);
    }
  }
  else {
  //  direccion == FLECHA_ABAJO 
    if ( list_it_is_last(&posic) )
      printf("\a");
    else {
        list_it_next(&posic);
      	aux = strdup(list_it_element(&posic));
	while ( !list_it_is_last(&posic) && !strsub(aux,patron)){
	  list_it_next(&posic);
	  free(aux);
      	  aux = strdup(list_it_element(&posic));
	}
	free(aux);
	aux = strdup(list_it_element(&posic));
	if (strsub(aux,patron)){
	   orden = strdup(list_it_element(&posic));
	}
	else {
	  printf("\a");
	  free(aux);
	  return NULL;
	}
	free(aux);
    }
  }
  list_it_copy(&posicion,&posic);
  return orden ;
}

void user_nueva_orden(char * orden){
        /* Implementación */
  char *null = (char *)malloc(sizeof(char)) ;
  null[0] = '\0';
  if ( strlen(orden)> 0 ) {
     free(ultima_orden);
     ultima_orden = strdup(orden);
     list_erase(&historia,orden) ; /////////////
     if ( list_size(&historia) > MAX_HISTORIA ){
        list_pop_front(&historia) ;
     }
     list_pop_back (&historia) ;
     list_push_back(&historia,orden) ;
     list_push_back(&historia, null) ;
     list_it_copy(&posicion,list_last(&historia)) ;
  }
}

//////////////////////////////////////////////////////////////////////////////////////
// [ 6 ] /////////    FIN DE FUNCIONES DE LA HISTORIA DE ORDENES    //////////////////
//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// [ 7 ] ////////    INICIO DE FUNCIONES DE LA CARACTERISTICA LS    //////////////////
//////////////////////////////////////////////////////////////////////////////////////
//										    //
//  Esquema general de la implementacion de la caracteristica ls:		    //
//										    //
//  Funciones:									    //
//										    //
//  Principal:									    //
//               int   user_ls(...);						    //
//  Secundarias:								    //
//		 int   aux_ls_directorio(...);					    //
//               int   aux_ls_ficheros  (...);					    //
//		 char *aux_ls_i(...);						    //
//               char *aux_ls_l(...);						    //
//               void  aux_ls_mostrar(...);					    //
//               int   aux_ls_errores(...);					    //
//  Auxiliares:									    //
//               int   comparador_ultima_palabra(...);				    //
//               int   comparador_con_Mayusculas(...);				    //
//               int   opciones_con_menos (...);				    //
//		 char**ordenar_en_columnas(...);				    //
//										    //
//    A continuacion se implementan las funciones principal y secundarias	    //
//  las funciones  auxiliares estan  implementadas al  principio  de este	    //
//  modulo.									    //
//										    //
//////////////////////////////////////////////////////////////////////////////////////


static char *aux_ls_i(char *dat,struct stat buf){
	int leng;
	char *solucion;
	char *numero;
	ino_t i = 0;
	i = buf.st_ino;
	numero = strbri((int)i,7,' ');
	leng = strlen(numero) + strlen(dat) + 2;
	solucion = (char *)malloc(sizeof(char)*(leng));
	sprintf(solucion,"%s %s",numero,dat);
	free(dat);
	return solucion;
} 

static char *aux_ls_l(char *dat,struct stat buf ){
	char *solucion;
	char *referencias;
	char *usuario;
	char *grupo;
	char *size;
	char fecha[14],fech_ult[6],permisos[11];
	int  lengh, bits_permisos = buf.st_mode;
	int  i, meses, segundos;
	time_t t;
	struct tm *tp;
	time(&t);
	tp = localtime(&t);
	meses = 12 * tp->tm_year + tp->tm_mon;
	segundos = t;
	permisos[10] = 0;
	// para saber si se trata de un directorio
	if ( ( bits_permisos & S_IFMT) == S_IFDIR )
	   permisos[0] = 'd';
	else
	   permisos[0] = '-';
	for ( i = 8 ; i >= 0 ; i-- ){
	   if ( bits_permisos % 2 )
	      permisos[i+1] = permisos_c[(8-i)%3];
	   else permisos[i+1] = '-'; 
	   bits_permisos /= 2; 
	} 
	referencias = strbri((int) buf.st_nlink,4,' ');
	usuario = strdup((getpwuid(buf.st_uid))->pw_name);
	grupo   = strdup((getgrgid(buf.st_gid))->gr_name);
	usuario = strers(usuario,8,' ');
	grupo   = strers( grupo ,8,' ');
	size = strbri((int) buf.st_size,8,' ');
	t = buf.st_mtime;
	tp = localtime(&t);
	strcpy(fecha, mes_cadena[ tp->tm_mon ]);
	strcat(fecha,strbri(tp->tm_mday,3,' '));
	strcat(fecha," ");
	meses -= 12 * tp->tm_year + tp->tm_mon;
	segundos -= t;
	// 6 meses.
	if ( ( meses < 6) && ( segundos > -3600 ) ){
	   sprintf(fech_ult,"%d%d:%d%d",
	          ( tp->tm_hour / 10),
		  ( tp->tm_hour % 10),
	          ( tp->tm_min  / 10),    
		  ( tp->tm_min  % 10));
	}
	else{
	   sprintf(fech_ult," %d",(1900 + tp->tm_year));   
	}
	strcat(fecha,fech_ult);
	lengh = strlen(referencias) + strlen(usuario) + strlen(grupo) +
	        strlen(size)  +  strlen(fecha)  +  strlen(dat)  +  17;
	solucion = (char *)malloc(sizeof(char)*(lengh));
	sprintf(solucion ,"%s %s %s %s %s %s %s",permisos,
	        referencias,usuario,grupo,size,fecha,dat);
	free(referencias);
	free(usuario);
	free(grupo);
	free(size);
	free(dat);
	return solucion;
}

static int aux_ls_mostrar(list_t *lista,int opc_l,
                          int opc_i, int son_fich){
	char **ficheros,**aux;
	int  size, i;
	size = list_size(lista);
	if ( size < 1 )
	   return 0;
	ficheros = list_to_array(lista);
	if ( opc_i || opc_l )
	   aux = ordenar_en_columnas(ficheros,size,comparador_ultima_palabra);
	else
	   aux = ordenar_en_columnas(ficheros,size,comparador_con_mayusculas);
	if ( opc_l  && !son_fich)
	   printf("Total %d\n",size);
	i = 0;
	while ( aux[i] ){
	   printf("%s\n",aux[i]);
	   free(aux[i]);
	   i++;
	}
	while ( list_size(lista) > 0 )
	   list_pop_front(lista);
	for (i = 0 ; i < size ; i++)
	   free(ficheros[i]);
	free(aux);
	free(ficheros);
	return 1;
}

int user_ls(char ** comandos){
	int  i = 0;
	int lenght;
	int opcion;
	int result;
	int print = 0;
	int opc_i = 0; 
	int opc_l = 0;
	int opc_a = 0;
	int num_parametros;
	char *direc = NULL;
	char opc[MAX_OPCIONES];
	// devuelve el numero de argumentos que no son una opcion
	num_parametros = opciones_con_menos(comandos,opc);
	while ( opc[i] ){
	   if ( opc[i] == 'a' )
	      opc_a = 1;
	   else if ( opc[i] == 'i' )
	      opc_i = 1;
	   else if ( opc[i] == 'l' )
	      opc_l = 1;
	   else {
	      printf("\aERROR: ls: -- %c: ",opc[i]);
	      printf("Opcion invalida \n");
	      printf("opciones validas: [-a] [-i] [-l]\n");
	      return 1;
	   }
	   i++;
	}
	opcion = 4*opc_l + 2*opc_i + opc_a;
	if ( num_parametros == 1 ){
	   result = aux_ls_directorio("./",opcion,0);
	   return (result == -1);
	}
	result = aux_ls_ficheros(comandos,opcion,num_parametros);
	i = 1;
	while ( comandos[i] ) i++;
	if ( i > 2 )
	   strQuickSort(comandos+1,i-1,comparador_con_mayusculas);
	if ( result != -1 )
	   print = result;
	i = 1;
  	while ( comandos[i] ){
	   if ( comandos[i][0] != '-'){
	      if ( comandos[i][strlen(comandos[i])-1] != '/' ) {
	        direc = (char *)malloc(sizeof(char)*(strlen(comandos[i])+2));
	        strcpy(direc,comandos[i]);
	        strcat(direc,"/");
	      }
	      else
	        direc = strdup(comandos[i]);
	      if ( num_parametros > 2 ) {
	         if ( print )
	            printf("\n");
		 printf("%s:",comandos[i]);
	      }
	      result = aux_ls_directorio(direc,opcion,(num_parametros > 2 ));
	      free(direc);
	      if ( result == 0 ){
		lenght = strlen(comandos[i]) + 2;
		direc = strers("1",lenght,' ');
	        printf("\r%s\r \b",direc);
		free(direc);
	      } else print = 1; 
	      
	   }
	   i++;
	}
	return TERMINA_BIEN;
}

static int aux_ls_directorio(char *direc,int opc,int endl){
   DIR *dir;
   char *datos ;
   int  opc_a,  opc_i ;
   int  opc_l, lenght ;
   char  *nombre_fich ;
   struct dirent  *dp ;
   struct stat    buf ;
   list_t  lista_fich ;
   opc_a = opc % 2;
   opc_i = ( opc / 2 ) % 2;
   opc_l = ( opc / 4 ) % 2;
   if ((dir = opendir(direc)) != NULL){
      list_new(&lista_fich) ;
      while((dp=readdir(dir))!=NULL){
	 // opcion -a
	 if ( dp->d_name[0] != '.' || opc_a ) {
	    
	    lenght = strlen(direc)+strlen(dp->d_name)+1;
	    nombre_fich = (char*)malloc(sizeof(char)*lenght);
	    strcpy(nombre_fich,direc);
	    strcat(nombre_fich,dp->d_name);
	    
	    if (stat(nombre_fich,&buf) != -1) {	    
	       datos = strdup(dp->d_name);
	       // opcion -l
	       if ( opc_l )
	          datos = aux_ls_l(datos,buf);
	       // opcion -i
	       if ( opc_i )
	          datos = aux_ls_i(datos,buf);
	       list_push_back(&lista_fich,datos);
	       free(datos);
	    }
	    else {
	       aux_ls_errores(nombre_fich);
	    }
	 }
      }
      closedir(dir);
      if ( list_size(&lista_fich) == 0 ){
         list_delete(&lista_fich);
	 return 0;
      }
      if (endl) printf("\n");
      aux_ls_mostrar(&lista_fich,opc_l,opc_i,0);
      list_delete(&lista_fich);
      return 1;
   }
   else
      return aux_ls_errores(direc);
}

static int aux_ls_ficheros(char **comandos,int opc,int size){
   int i = 1, j = 1, muestra = 0;
   int opc_a, opc_i, opc_l ;
   char *datos;
   char **comand =(char**)malloc(sizeof(char*)*(size+1));
   struct stat buf ;
   list_t lista_fich ;
   opc_a = opc % 2;
   opc_i = ( opc / 2 ) % 2;
   opc_l = ( opc / 4 ) % 2;
   list_new( &lista_fich );
   while ( comandos[i] ) {
      if ( comandos[i][0] == '-'){
         if (comandos[i][1] == '-' ){
	    printf("\aERROR: ls: '%s': ",comandos[i]);
	    printf("Opcion invalida \n");
	    printf("opciones validas: [-a] [-i] [-l]\n");
	    return -1;
	 }
      }
      else if (stat(comandos[i],&buf) == -1) {
         aux_ls_errores(comandos[i]);
      }
      else{ 
         if ( ( buf.st_mode & S_IFMT ) == S_IFDIR) {
	    comand[j] = comandos[i];
	    j++;
	 }
	 else {
	    if ( ( comandos[i][0] != '.') || 
	         opc_a || strsub(comandos[i],"/") ) {
	       datos = strdup(comandos[i]);
	       // opcion -l
	       if ( opc_l )
	          datos = aux_ls_l(datos,buf);
	       // opcion -i
	       if ( opc_i )
		  datos = aux_ls_i(datos,buf);
	       list_push_back(&lista_fich,datos);
	       free(datos);
	    }
	    free(comandos[i]);
	 }
      }
      i++;
   }
   comand[j] = NULL;
   muestra = (  list_size(&lista_fich) > 0 );
   if ( muestra )
      aux_ls_mostrar(&lista_fich,opc_l,opc_i,1);
   list_delete(&lista_fich);
   j = 1;
   while ( comand[j] ){
      comandos[j] = comand[j];
      j++;
   }
   comandos[j] = NULL;
   free(comand);
   return muestra;
}

static int aux_ls_errores(char *fichero){
  switch( errno ){
     case ENOTDIR: printf("\r\aERROR: ls: %s: No es un directorio\n",fichero); break;
     case EACCES : printf("\r\aERROR: ls: %s: Permiso denegado\n",fichero);    break;
     case ENOENT : printf("\r\aERROR: ls: %s:",fichero);
                   printf(" No existe el fichero o el directorio\n");          break;
     default     : printf("\r\aERROR: ls: %s: No se pudo abrir\n",fichero);    break;
  }
  return 0;
}

//////////////////////////////////////////////////////////////////////////////////////
// [ 7 ] //////////    FIN DE FUNCIONES DE LA CARACTERISTICA LS    ///////////////////
//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// [ 8 ] //    INICIO DE FUNCION PAR IMPLEMENTAR LOS SEPARADORES AND Y OR    /////////
//////////////////////////////////////////////////////////////////////////////////////


int aux_or_and_sintaxis(char **comandos){
  int i = 0;
  if ( ( strcmp(comandos[0],AND_S) == 0) ||
       ( strcmp(comandos[0], OR_S) == 0)  ) {
	printf("\aERROR: smallsh: %s:",(1+comandos[0]));
	printf(" Se esperaba un token inicial\n");
	return 0;
   }
   while (comandos[i]){
      if ( ( ( strcmp(comandos[i],AND_S) == 0) ||
	     ( strcmp(comandos[i], OR_S) == 0) ) &&
	   ( ( strcmp(comandos[i-1],AND_S) == 0) ||
	     ( strcmp(comandos[i-1], OR_S) == 0) ) ){
	  printf("\aERROR: smallsh: %s:",(1+comandos[0]));
	  printf(" Se esperaba un token despues del operador\n");
	  return 0;     
      }
      i++;
   }
   if ( ( strcmp(comandos[i-1],AND_S) == 0) ||
	     ( strcmp(comandos[i-1], OR_S) == 0)  ) {
      printf("\aERROR: smallsh: %s:",(1+comandos[i-1]));
      printf(" Se esperaba un token despues del operador\n");
      return 0;
   }
  return 1;
}

int user_or_and(char **comandos){
   int   terminar = 0;
   int   resultado;
   int   j, i = 0,  pid;
   char *comand[MAXARG];
   
   if ( !aux_or_and_sintaxis(comandos) )
      return TERMINA_MAL;
   while ( !terminar && comandos[i] ){
      j = 0;
      while ( ( comandos[i] != NULL ) &&
              strcmp(comandos[i], OR_S) &&
              strcmp(comandos[i],AND_S)  )
	  comand[j++] = comandos[i++];
      comand[j++] = NULL;
      // la ejecucion de esta orden en segundo plano o con @ es general
      // no de cada una de sus sub ordenes.
      if ( smallsh_comand(comand,OP_INIT) ){
	 resultado = exec_smallsh(comand,OP_INIT);
      }
      else if ( (pid = fork()) == 0 ){
	 execvp( comand[0], comand);
         printf("\aERROR: smallsh: %s:",comand[0]);
         printf(" Comando no encontrado\n");
	 exit(-1);
      }
      else{
         waitpid(pid,&resultado,0);
      }
      if ( comandos[i] ) {
         if ( !strcmp(comandos[i], OR_S) && resultado == 0 )
            terminar = 1; 
         if ( !strcmp(comandos[i],AND_S) && resultado != 0 )
            terminar = 1;
         i++;
      }
   }
   return resultado;
}

//////////////////////////////////////////////////////////////////////////////////////
// [ 8 ] ///    FIN DE FUNCION PAR IMPLEMENTAR LOS SEPARADORES AND Y OR    ///////////
//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// [ 9 ] //////    INICIO DE FUNCIONES DE LA  CARACTERISTICA ARROBA    ///////////////
//////////////////////////////////////////////////////////////////////////////////////
//										    //
//  Esquema general de la implementacion de la caracteristica arroba:		    //
//										    //
//  Funciones:									    //
//										    //
//  Principal:									    //
//		 void user_arroba(...);						    //
//  Secundaria:									    //
//		 void sig_alarm_handler(...);					    //
//										    //
//    A continuacion se implementan estas dos funciones.			    //
//////////////////////////////////////////////////////////////////////////////////////


static void sig_alarm_handler(int sig){
  longjmp( funcion_arroba, 1);
}

void user_arroba( unsigned int tiempo,char *comando){
   int estado, pid = getpid();
   if ( tiempo == 0 ) {
      printf("smallsh: %s: Proceso:",comando);
      printf(" [%d] tiempo %d excedido \n",pid,tiempo);
      exit(-1);
   }
   //    El proceso padre se queda en esta funcion hasta que termine
   // el proceso  hijo, el cual es el que  continua con la ejecucion
   // en runcomand.
   if ( ( pid = fork() ) != 0 ) {
      if ( signal(SIGALRM,sig_alarm_handler) == SIG_ERR ) {
         perror("Signal");
         exit(-1);
      }
      if ( setjmp(funcion_arroba) == 0 ) {
         alarm(tiempo);
	 while ( waitpid(pid,&estado,0) != -1)
	   	;
	 alarm(0);
	 exit(estado);
      } else {
         //   No hay  que arriesgarse  a que el 
	 // proceso hijo pueda ignorar al padre
         kill(pid,SIGKILL);
	 printf("\r                                        ");
	 printf("                                        \r");
         printf("smallsh: [ Proceso %d ]: ",getpid());
         printf(" %s: Tiempo %d excedido\n", comando, tiempo);
	 exit(TERMINA_MAL);
      }
   }
}

//////////////////////////////////////////////////////////////////////////////////////
// [ 9 ] ////////    FIN DE FUNCIONES DE LA CARACTERISTICA ARROBA    /////////////////
//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// [ 10 ] //////    INICIO DE FUNCIONES PARA EL MANEJO DE VARIABLES    ///////////////
//////////////////////////////////////////////////////////////////////////////////////
//										    //
//  Esquema general de la implementacion para el manejo de variables:		    //
//										    //
//  Funciones:									    //
//										    //
//  Principales:								    //
//		 char *user_variable  (...);					    //
//		 int   user_definicion(...);					    //
//		 int   user_set   (...);					    //
//		 int   user_unset (...);					    //
//		 int   user_export(...);					    //
//  Secundarias:								    //
//		 int   aux_variables_es_definicion(...);			    //
//		 int   aux_variables_aislar_nombre_final(...);			    //
//		 int   aux_variables_aislar_nombre_principio(...);		    //
//		 char *aux_variables_cambiar_variables(...);			    //
//										    //
//    A continuacion se implementan estas funciones.				    //
//										    //
//////////////////////////////////////////////////////////////////////////////////////


static char *aux_variables_get(char *nom){
   int leng = strlen(nom) + 2;
   char *elemento, *nombre, *valor = NULL;
   list_it it;
   if ( list_size(&variables) == 0)
      return NULL;
   nombre = (char *)malloc(sizeof(char)*leng);
   sprintf(nombre,"%s=",nom);
   list_it_copy(&it,list_first(&variables));
   elemento = list_it_element(&it);
   leng--;
   while ( !list_it_is_last(&it) && 
           strncmp(elemento,nombre,leng) ){
      list_it_next(&it);
      elemento = list_it_element(&it);	   
   }
   if (!strncmp(elemento,nombre,leng))
      valor = strdup(elemento+leng);
   free(nombre);
   return valor;
}

static int aux_variables_erase(char *nom){
   int leng, res = 0;
   char *elemento, *nombre, *auxiliar;
   list_it it;
   if ( list_size(&variables) == 0)
      return 0;
   leng = strlen(nom) + 2;
   nombre = (char *)malloc(sizeof(char)*leng);
   sprintf(nombre,"%s=",nom);
   list_it_copy(&it,list_first(&variables));
   elemento = list_it_element(&it);
   leng--;
   while ( !list_it_is_last(&it) && 
           strncmp(elemento,nombre,leng) ){
      list_it_next(&it);
      elemento = list_it_element(&it);	   
   }
   auxiliar = strdup(elemento);
   if (!strncmp(elemento,nombre,leng)){
      list_erase(&variables,auxiliar);
      res = 1;
   }
   free(auxiliar);
   free(nombre);
   return res;
}

static int aux_variables_es_definicion(char *entrada){
   int i = 1;
   if ( !entrada || !isalpha(entrada[0]) )
      return 0;
   while ( isalnum(entrada[i]) ) i++;
   return (entrada[i] == '=');
}

static int aux_variables_aislar_nombre_final(char **entrada){
   int i = 1;
   char *auxiliar;
   if  ( entrada[0][0] == '?' ){
      entrada[2] = strdup(entrada[0]+1);
      entrada[1] = strdup("?");
      return 1;
   }
   if ( !isalpha(entrada[0][0])  ) {
      entrada[2] = strdup(entrada[0]);
      return 0;
   }
   while ( isalnum ( entrada[0][i] ) ) i++;
   auxiliar = entrada[0]+i;
   entrada[2] = strdup( auxiliar);
   auxiliar  = strdup(entrada[0]);
   auxiliar[i] = 0;
   entrada[1] = strdup( auxiliar);
   free(auxiliar);
   return 1;
} 

static int aux_variables_aislar_nombre_principio(char **entrada){
   char *auxiliar;
   if ( entrada[0][0] == '$' ){
      entrada[1] = strdup("");
      entrada[2] = strdup(entrada[0]+1);
      return 1;
   }
   else if ( strsub(entrada[0],"$") ){
         auxiliar   = strdup( entrada[0] );
	 entrada[1] = strdup(strtok(auxiliar, "$"));
         entrada[2] = strdup(strtok(  NULL  ,"\0"));
         free(auxiliar);
         return 1;
   }
   else return 0;
}

static char *aux_variables_cambiar(char *entrada,int *pnum_cambios,list_t *l){
   char  *previo, *posterior;
   char **auxiliar,*valor,*resultado;
   int leng;
   
   auxiliar = (char **)malloc(sizeof(char*)*3);
   auxiliar[0] = entrada;
   if ( !aux_variables_aislar_nombre_principio(auxiliar) ){
      free(auxiliar);
      return strdup(entrada);
   }
   previo = auxiliar[1];
   auxiliar[0] = auxiliar[2];
   if  ( !aux_variables_aislar_nombre_final(auxiliar) )
      valor = strdup("$");
   else{
      valor = aux_variables_get(auxiliar[1]);  
      if ( valor == NULL ){
	 if ( getenv(auxiliar[1]) == NULL ){
	    leng = strlen(auxiliar[1]) + 2;
	    valor = (char *)malloc(sizeof(char)*leng);
	    sprintf(valor,"$%s",auxiliar[1]);
	 }
	 else {
	    valor = strdup(getenv(auxiliar[1]));
	    *pnum_cambios += 1;
	    list_insert(l,auxiliar[1]);
	 }
      }
      else {
         *pnum_cambios += 1;
         list_insert(l,auxiliar[1]);
      }
   }
      
   posterior = aux_variables_cambiar(auxiliar[2],pnum_cambios,l);
   leng = strlen(previo)+strlen(valor)+strlen(posterior)+1;
   resultado = (char *)malloc(sizeof(char)*leng);
   sprintf(resultado,"%s%s%s",previo,valor,posterior);
   free(previo);
   free(valor);
   free(posterior);
   free(auxiliar[0]);
   free(auxiliar[1]);
   free(auxiliar[2]);
   free(auxiliar);
   return resultado;
}

int aux_var_comp(list_t *l1,list_t *l2){
  int i, size = list_size(l1);
  char **t1 = list_to_array(l1);
  char *e;
  while ( list_size(l2) > 0 ) {
     e = list_front(l2);
     i = 0;
     while ( ( i < size ) && ( strcmp(t1[i],e) < 0 ) )
        i++;
     if ( ( i < size ) && ( strcmp(t1[i],e) == 0 ) )
       return 1;
     list_insert(l1, e);
     list_pop_front(l2);
  }
  return 0;
}

char  *user_variables(char *entrada){
	/* Implementacion */
   int  salir = 0;
   int   num_cambios = 0;
   int total_cambios = 0;
   char *resultado;
   list_t lista1;
   list_t lista2;
   list_new(&lista1);
   list_new(&lista2);
   resultado = aux_variables_cambiar(entrada,&num_cambios,&lista1);
   while ( num_cambios > 0 && !salir ){
      total_cambios += num_cambios;
      num_cambios = 0;
      resultado = aux_variables_cambiar(resultado,&num_cambios,&lista2);
      salir = aux_var_comp(&lista1,&lista2);
   }
   if ( total_cambios > 0 ) {
      if ( salir == 1 )
	 return NULL;
      else
         return resultado;
   }
   else {
      free(resultado);
      return NULL;
   }
}

int user_definicion(char **def){
	/* Implementacion */
	char *definicion;
	char *nombre;
	char *valor;
	if ( def[1] ){
		printf("\aERROR: smallsh: Sintaxis erronea\n");
		return TERMINA_MAL;
	}
	else {
		definicion = strdup(def[0]);
		nombre = strtok(definicion, "=");
		valor  = strtok(   NULL   ,"\0");
		// para evitar tener variables duplicadas
		if  (   ( aux_variables_get(nombre) == NULL )&&
			( getenv(nombre) != NULL )    )
			setenv(nombre,(const char*)valor,1);
		else {
		   aux_variables_erase(nombre);
		   list_insert(&variables,def[0]);
		}
		return TERMINA_BIEN;
	}
}

int  user_set(char *s){
	if ( s ){
	   printf("\aERROR: set: No utiliza parametros\n");
	   return TERMINA_MAL;
	}
	list_it it;
	if ( list_size(&variables) > 0 ){
	   list_it_copy(&it,list_first(&variables));
	   while ( !list_it_is_last(&it) ){
	      printf("%s\n",list_it_element(&it));
	      list_it_next(&it);
	   }
	   printf("%s\n",list_it_element(&it));
	   return TERMINA_BIEN;
	}
	else
	   return TERMINA_MAL;
}

int  user_unset(char **variables){
	int i = 1;
	while ( variables[i] != NULL ) {
	   if (aux_variables_erase(variables[i]));
	   else if (getenv(variables[i]) != NULL)
	      unsetenv(variables[i]);
	   else {
	   	printf("\aERROR: unset: %s: No existe\n",variables[i]);
		return TERMINA_MAL;
	   }
	   i++;
	}
	if ( i == 1 ){
	   printf("\aERROR: unset: Necesita ");
	   printf("almenos el nombre de una variable\n");
	   return TERMINA_MAL;
	}
	else
	   return TERMINA_BIEN;
}

int  user_export(char **variables){
	int i = 1;
	char *valor; 
	while (variables[i] != NULL){
	   valor = aux_variables_get(variables[i]);
	   if ( valor ){
	      setenv(variables[i],valor,1);
	      aux_variables_erase(variables[i]);
	   }
	   else{
	      printf("\aERROR: export: %s: ",variables[i]);
	      printf("No es una variable local\n");
	      return TERMINA_MAL;
	   }
	   i++;
	}
	if ( i == 1 ){
	   printf("\aERROR: export: Necesita almenos ");
	   printf("el nombre de una variable local\n");
	   return TERMINA_MAL;
	}
	else
	   return TERMINA_BIEN;
}

//////////////////////////////////////////////////////////////////////////////////////
// [ 10 ] ///////    FIN DE FUNCIONES PARA EL MANEJO DE VARIABLES    /////////////////
//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// [ 11 ] ///////////////    INICIO DE FUNCION ORDEN EXIT    /////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//										    //
//  Esta caracteristica solo necesita la funcion: int user_exit(...);		    //
//						    	    			    //
//////////////////////////////////////////////////////////////////////////////////////


int user_exit(){

   comand_exit();
   kill(0,SIGTERM);
   bbdd_process_delete(&bdP);
   return TERMINA_BIEN;
}

//////////////////////////////////////////////////////////////////////////////////////
// [ 11 ] ////////////////    FIN DE FUNCION ORDEN EXIT    ///////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// < MEJORA 1 > ///   INICIO DE FUNCIONES PARA EL USO DEL TABULADOR   ////////////////
//////////////////////////////////////////////////////////////////////////////////////

static int  aux_tab_variables(list_t *l,char *patron){
   char *e;
   int leng = strlen(patron);
   list_it it;
   if ( list_size(&variables) > 0 ) {
      list_it_copy(&it,list_first(&variables));
      while ( !list_it_is_last(&it) &&
               strcmp(list_it_element(&it),"?") != 0){
         e = strtok(strdup(list_it_element(&it)),"=");
	 if ( strncmp(e,patron,leng) == 0 )
	    list_insert(l,e);
	 list_it_next(&it);
	 free(e);
      }
      e = strtok(strdup(list_it_element(&it)),"=");
      if ( ( strncmp(e,patron,leng) == 0 ) &&
             strcmp(list_it_element(&it),"?") != 0){
	 list_insert(l,e);
      }
      free(e);
      return 1;
   }
   else
      return 0;
}

static void aux_tab_comandos_internos(list_t *l,char *patron){
    int i = 0;
    int leng = strlen(patron);
    while ( i < COMANDOS_INT ){
       if ( strncmp(comandos_internos[i],patron,leng) == 0 ){
	  list_insert(l,comandos_internos[i]);
       }
       i++;
    }
}

static void aux_tab_dir(list_t *l,char *dir_name, char *patron,int opcion){
   DIR *dir;
   int comparar = strlen(patron);
   int   es_dir, es_eje;
   char *directorio;
   char *fichero;
   char *solucion;
   struct dirent *dp;
   struct stat   stt;
   int leng = strlen(dir_name);
   if ( dir_name[leng-1] == '/' )
      directorio = strdup(dir_name);
   else {
      directorio = (char *)malloc(sizeof(char)*(leng+2));
      sprintf(directorio,"%s/",dir_name);
   }
   if ( (dir = opendir(directorio)) != NULL ){
      while ( ( dp = readdir(dir) ) != NULL ){
       
         if ( ( strcmp (dp->d_name,"." ) != 0 ) && 
	      ( strcmp (dp->d_name,"..") != 0 ) &&
	      ( strncmp(dp->d_name,patron,comparar) == 0 )){
            leng = strlen(directorio)+strlen(dp->d_name)+2;
	    fichero = (char *)malloc(sizeof(char)*leng);
	    sprintf(fichero,"%s%s",directorio,dp->d_name);
	    if ( stat(fichero, &stt) != -1 ){
	       es_dir = es_eje = 0;
	       if ((stt.st_mode / 4096) == 4)
		  es_dir = 1;
	       if (!access(fichero,X_OK))
		  es_eje = 1;
	       if ( es_dir || es_eje || opcion ){
	          leng = strlen(dp->d_name)+2;
		  solucion = (char *)malloc(sizeof(char)*leng);
		  if ( es_dir )
		     sprintf(solucion,"%s/",dp->d_name);
		  else
		     sprintf(solucion,"%s",dp->d_name);
		  list_insert(l,solucion);
		  free(solucion);
	       }
	    }
	    free(fichero);
	 }
      }
      closedir(dir);
   }
}

int aux_tab_segunda_vez(list_t *soluciones){
   char c;
   char **t_aux, **t;
   int i, j;
   int valida;
   int mostrar = 1;
   int lenght = list_size(soluciones);
   if ( lenght > 50 ){
      valida = 0;
      printf("\n");
      while ( ! valida ) {
	 printf("\rHay %d ",lenght);
         printf("posibilidades, ¿ desea continuar ? ( s / n )");
	 scanf("%c",&c);
	    if ( c == 's' || c == 'n'){
	       valida = 1;
	       mostrar = ( c == 's' ? 1 : 0 );
	    }
	    else
	       printf("\a");
      }
   }
   printf("\n");
   if ( mostrar ){
      t = list_to_array(soluciones);
      t_aux = ordenar_en_columnas(t,lenght,comparador_con_mayusculas);
      lenght = 0;
      while ( t_aux[lenght] ) lenght++;
        if ( lenght > 50 ){
	   i = 25;
	   j = 0;
           while ( lenght > j ){
  	      while ( i > 0 && t_aux[j] ){
		 printf("%s\n",t_aux[j]);
		 free(t_aux[j]);
		 j++;
		 i--;
	      }
	      printf("-- Mas --");
	      scanf("%c",&c);
	      printf("\r         \r");
	      switch( c ){
		 case 'q' : while ( t_aux[j] )
			    {  free(t_aux[j]); j++;}
			    lenght = list_size(soluciones);
			    for ( i = 0 ; i < lenght ; i++)
			    {   free(t[i]); }
			    free(t_aux);
			    free(t);
			    return 1;     break;
		 case ' ' : i = 20;       break;
		 case '\n': i =  1;       break;
		 default  : printf("\a"); break;
	      }
	   }
	}
	else{
	   i = 0;
	   while ( t_aux[i] ){
	      printf("%s\n",t_aux[i]);
	      free(t_aux[i]);
	      i++;
	   }
	}
	for ( i = 0 ; i < lenght ; i++)
	   free(t[i]);
	   free(t_aux);
	   free(t);
	}
	return 1;
}

char *aux_tab_es_variable(char *patron){
  int i = 0;
  char *c, *aux;
  if ( strlen(patron) > 0 ){
    if ( ( c = strrchr(patron,'$')) != NULL ){
       i = 1;
       if ( c[i] && !isalpha(c[i]) ) return NULL;
       aux = strdup(c+1);
       while ( c[i] && isalnum(c[i])) i++;
       if ( c[i] )
          return NULL;
       else
          return aux;
    }
    return NULL;
  }
  return NULL;
}

char *aux_tab_sintaxis(char *parte,int *pcomillas){
    int i = 0, j = 0;
    int comillas = 0;
    char *patron = strdup(parte);
    if ( aux_variables_es_definicion(parte) )
      patron = 1 + (strchr(patron,'='));
    if ( patron[0] ) {
       while ( patron[i] ){
          if ( patron[i] != '\\' && patron[i] != '\'' ) {
	     patron[j++] = patron[i];
	  }
	  else if ( patron[i+1] ) {
	     if ( patron[i] == '\'' )
	        comillas = !comillas;
	     i++;
	     patron[j++] = patron[i];
	  }
	  else if ( patron[i] == '\'' )
	    comillas = !comillas;
	  i++;
       }
    }
    *pcomillas = comillas;
    patron[j] = 0;
    return patron;
}

char * aux_tab_restaura_sintaxis(char * parte,int comillas){
  int i = 0;
  int cont = strlen(parte)+1;
  char *resultado;
  if ( !comillas ) {
     while ( parte[i] ){
        if ( !isalnum(parte[i] ) && parte[i] != '/' &&
	      parte[i] != '.' )
           cont++;
        i++;
     }
     resultado = (char *)malloc(sizeof(char)*cont);
     i = 0;
     cont = 0;
     while ( parte[i] ){
        if ( !isalnum(parte[i] ) && parte[i] != '/' &&
	      parte[i] != '.' )
           resultado[cont++] = '\\';
        resultado[cont++] = parte[i++];
     }
     resultado[cont++] = 0;
  }
  else {
     cont++;
     resultado = (char *)malloc(sizeof(char)*cont);
     sprintf(resultado,"%s'",parte);
     if ( resultado[cont-3] == '/' ){
        resultado[cont-3] = '\'';
	resultado[cont-2] = '/';
     }
  }
  return resultado;
}

char *user_tabulador(char * parte_recibida, int numero, int numtab) {
	list_t soluciones;
	int  hay_comillas;
	int  leng;
	char *var;
	char *aux;
	char *ruta, *auxiliar;
	char *parte = parte_recibida;
	char *parte_final,  *solucion;
        char *directorio;
	list_new(&soluciones);
	parte = aux_tab_sintaxis(parte_recibida,&hay_comillas);
	if ( (var = aux_tab_es_variable(parte)) != NULL ){
	   parte_final = strdup(var);
	   ruta = strdup("");
	   aux_tab_variables(&soluciones,var);
	}
        else {
	   if ( ( aux = user_variables(parte) ) != NULL )
	     parte = aux;
	   // si no se ha introducido una ruta	
	   if( !strsub(parte,"/") ) {
	      parte_final = strdup(parte);
	      if( numero == 1)
	         ruta = strdup(getenv("PATH"));
	      else
	         ruta = strdup("./");
	   }
	   else {
	      ruta = strdup(parte);
	      if ( parte[strlen(parte)-1] == '/' )
	         parte_final = strdup("");
	      else {
	         auxiliar = strrchr(ruta,'/');
	         auxiliar[1] = 0;
	         parte_final = strdup(strrchr(parte,'/')+1);
	      }
	   }
           directorio = strtok(ruta,":");
	   while ( directorio != NULL ) {
	      aux_tab_dir(&soluciones,directorio,parte_final,numero>=2);
	      directorio = strtok( NULL ,":" );
	   }
	   if ( numero ==  1 && !strsub(parte,"/") )
	      aux_tab_comandos_internos(&soluciones,parte_final);
	}
	if ( list_size(&soluciones) == 0){
	   printf("\a");
	   free(ruta);
	   free(parte_final);
	   return NULL;
	}
	if ( numtab == 2 ){
	   aux_tab_segunda_vez(&soluciones);
	   return NULL;
	}
	else {
	   solucion = strdup(list_front(&soluciones));
	   list_pop_front(&soluciones);
	   if ( list_size(&soluciones) > 0) {
	      printf("\a");
	      while ( (list_size(&soluciones) > 0) &&
	              ( strcmp(solucion,parte_final) != 0) ){
	         auxiliar = strbeq(solucion,list_front(&soluciones));
		 free(solucion);
		 solucion = auxiliar;
	         list_pop_front(&soluciones);
	      }
	   }
	   leng = strlen(parte_final);
	   if ( strcmp(solucion,parte_final) == 0){
	      printf("\a");
	      free(ruta);
	      free(parte_final);
	      return NULL;
	   }
	   solucion = strdup(solucion+leng);
	   free(ruta);
	   free(parte_final);
	   return aux_tab_restaura_sintaxis(solucion,hay_comillas);
	}
}

//////////////////////////////////////////////////////////////////////////////////////
// < MEJORA 1 > ///   FIN DE FUNCIONES PARA EL USO DEL TABULADOR   ///////////////////
//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// < MEJORA 2 > ////////   INICIO DE FUNCIONES JOBS, BG Y FG   ///////////////////////
//////////////////////////////////////////////////////////////////////////////////////


int   user_jobs(char **c) {
   bbdd_process_show(&bdP,1);
   return TERMINA_BIEN;
}

int   user_bg  (char **c) {
   int id;
   process *p;
   if ( !c[1] ){
      printf("\aERROR: fg: Necesita el identificador");
      printf(" de un trabajo detenido\n");
      return TERMINA_MAL;
   }
   p  = NULL;
   if ( strton(c[1],&id) ){
      p = bbdd_process_get_process(&bdP,id);
   }
   if ( p != NULL ){
      printf("%s\n",p->orden);
      p->estado = Run;
      kill(p->pid,SIGCONT);
      return TERMINA_BIEN;
   }
   else {
      printf("\aERROR: fg: %s: ",c[1]);
      printf("No es el identificador de un proceso\n");
      return TERMINA_MAL;
   }
}

int   user_fg  (char **c) {
   int id, status, ret;
   process *p;
   if ( !c[1] ){
      printf("\aERROR: fg: Necesita el identificador");
      printf(" de un trabajo detenido\n");
      return TERMINA_MAL;
   }
   p  = NULL;
   if ( strton(c[1],&id) ){
      p = bbdd_process_get_process(&bdP,id);
   }
   if ( p == NULL ){
      printf("\aERROR: fg: %s: ",c[1]);
      printf("No es el identificador de un proceso\n");
      return TERMINA_MAL;
   }
   else{
      printf("%s\n",p->orden);
      p->estado = Run;
      kill(p->pid,SIGCONT);
      ret = waitpid(p->pid,&status,WUNTRACED);
      if ( WIFSTOPPED(status) ){
         printf("[%d]  %s\t%s\n",id,STOPPED,p->orden);
         p->estado = Stop;         
      }
   }
   return status;
}

void  user_jobs_done(){
   bbdd_process_show(&bdP,0);
}

void  user_nuevo_job(int pid,int estado){
   bbdd_process_insert(&bdP,pid,ultima_orden,estado);
}

//////////////////////////////////////////////////////////////////////////////////////
// < MEJORA 2 > /////////   FIN DE FUNCIONES  JOBS, BG Y FG   ////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// < MEJORA 3 > /////////   INICIO DE FUNCIONES TAIL Y HEAP   ////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//										    //
//  Esquema general de la implementacion de la mejora de las funciones tail y heap: //
//										    //
//  Funciones:									    //
//										    //
//  Principal:									    //
//               int  user_tail_heap(...);					    //
//  Secundarias:								    //
//               int  aux_tail(...);						    //
//               int  aux_heap(...);						    //
//										    //
//    A continuacion se implementas  estas dos funciones tienen un		    //
//  esquema muy similar.							    //
//										    //
//////////////////////////////////////////////////////////////////////////////////////


static int aux_tail(char *nombre_fichero, int numero_lineas){
   int size, i;
   char *bloque;
   FILE *fichero;
   struct stat *stbuf;
   stbuf = (struct stat *) malloc (sizeof(struct stat));
   if (stat(nombre_fichero,stbuf) != 0){
      printf("\aERROR: tail: %s: ",nombre_fichero);
      printf("No existe el fichero o el directorio\n");
      return 0;
   }
   else {
      fichero = fopen(nombre_fichero, "r");
   }
   size = stbuf->st_size;
   bloque =(char*)mmap(0,size,PROT_READ,MAP_SHARED,fileno(fichero),0);
   i = size -1;
   numero_lineas++;
   while (numero_lineas > 0 && i >= 0 ) {
      if (bloque[i] == '\n') {
	 numero_lineas--;
      }
      i--;
   }
   i++;
   if ( i != 0 )
      i++;
   while (i != size){
      printf("%c",bloque[i]);
      i++;
   }
   munmap(bloque,size);
   fclose(fichero);
   return 1;
}

static int aux_head(char *nombre_fichero, int numero_lineas){
   int size, i;
   char *bloque;
   FILE *fichero;
   struct stat *stbuf;
   stbuf = (struct stat *) malloc (sizeof(struct stat));
   if (stat(nombre_fichero,stbuf)){
      printf("\aERROR: head: %s: ",nombre_fichero);
      printf("No existe el fichero o el directorio\n");
      return 0;
   }
   size = stbuf->st_size;
   fichero = fopen(nombre_fichero, "r");
   bloque =(char*)mmap(0,size,PROT_READ,MAP_SHARED,fileno(fichero),0);
   i = 0;
   while (numero_lineas > 0 && i < size ) {
      if (bloque[i] == '\n') {
	 numero_lineas--;
      }
      printf("%c", bloque[i]);
      i++;
   }
   munmap(bloque,size);
   fclose(fichero);
   return 1;
}

int  user_tail_head(char **comandos,int tail){
	int numero_lineas = 10, j, k, result;
	char *nombre_fichero;
	if ( comandos[1] ) {
	   if ( !strcmp(comandos[1],"-n") ){
	   	if ( comandos[2] ){
	   	   if ( !strton(comandos[2],&numero_lineas) ){
		   	printf("\aERROR: %s: %s: ",comandos[0],comandos[2]);
	   		printf(" El numero de lineas no es valido\n");
	   		return TERMINA_MAL;
		   }
		   k = 3;
		}
		else {
		   printf("\aERROR: %s: La opcion '-n' ",comandos[0]);
		   printf("requiere un argumento numerico\n");
		   return TERMINA_MAL;
		}
	   }
	   else 
	      k = 1;
	   j = k;
	   while (comandos[j]) { j++;}
	   j -= k;
	   while ( comandos[k] ) {
	      if ( j > 1 )
	         printf("==> %s <==\n",comandos[k]);
	      nombre_fichero =  strdup(comandos[k]);
	      if ( tail )
	         result = aux_tail(nombre_fichero,numero_lineas);
	      else
	         result = aux_head(nombre_fichero,numero_lineas);
	      free(nombre_fichero);
	      k++;
	      if ( j > 1 && comandos[k] && result )
	         printf("\n");
	   }
	   if ( j > 0 )
	      return TERMINA_BIEN;
	   else {
	      printf("\aERROR: %s:",comandos[0]);
              printf("Nescesita un fichero\n");
	      return TERMINA_MAL;
	   }
	}
	else {
	   printf("\aERROR: %s:",comandos[0]);
           printf("Nescesita un fichero\n");
	   return TERMINA_MAL;
	}
}

//////////////////////////////////////////////////////////////////////////////////////
// < MEJORA 3 > //////////   FIN DE FUNCIONES  TAIL Y HEAP   /////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

/* $Id: userfn.c 910 2005-01-11 13:28:59Z dsevilla $ */
