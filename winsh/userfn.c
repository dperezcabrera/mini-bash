#include "winsh.h"
#include "list.h"
#include "string_ext.h"
#include <sys/stat.h>
#include <io.h>

#define MAX_DIR 2048
#define MAX_HISTORIA 1000 // maximo de  ordenes guardadas  en la  historia.
#define MAX_LINEA    4096 // maximo de  linea que  puede tener la historia.
#define MAX_OPCIONES  100 // maximo de opciones distintas que pueden tener.
#define LONG_COLUMNS   80 // numero de columnas.

//////////////////////////////////////////////////////////////////////////////////////
//  Definicion variables globales que se necesitaran para las caracteristicas.      //
//////////////////////////////////////////////////////////////////////////////////////

list_t variables ;
list_t  historia ;
list_it posicion ;

static int long_columns =  80;

//////////////////////////////////////////////////////////////////////////////////////
//  Definicion variables globales que no seran modificadas. ( las constantes )      //
//////////////////////////////////////////////////////////////////////////////////////

static char  mes_cadena[][4] = { "Ene", "Feb", "Mar", "Abr", "May", "Jun", 
													   "Jul", "Ago", "Sep", "Oct", "Nov", "Dic"};
static char  shell_order[][7] = 
         { "cd","clear", "echo", "env", "exit", "export", 
		    "head","ls" , "set" , "tail" , "unset", "see"};

//////////////////////////////////////////////////////////////////////////////////////
//  Definicion de funciones que se necesita saber que estaran implementadas.        //
//////////////////////////////////////////////////////////////////////////////////////

static void user_variables_init();
static void aux_historia_cargar ( );
static void aux_historia_guardar( );
static void user_signal_init();

//////////////////////////////////////////////////////////////////////////////////////
// ( E ) ///////////    INICIO DE OTRAS FUNCIONES EXPORTABLES    ///////////////
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
   columnas = LONG_COLUMNS / (longitud_mayor + 1) ;
   if ( columnas != 0 ) {
      filas = size / columnas;
      if ( size % columnas )
         filas++;
   }
   if ( columnas > 1 ){
      result = (char **)malloc(sizeof(char*)*(filas+1));
      for ( j = 0 ; j < filas ; j++){
         result[j] = (char *)malloc(sizeof(char)*(LONG_COLUMNS+1));
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
	char op[2];
	list_t lista;
	list_new(&lista);
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

//////////////////////////////////////////////////////////////////////////////////////
// ( E ) /////////////    FIN DE OTRAS FUNCIONES EXPORTABLES    /////////////////
//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// ( G ) ///////////////    INICIO DE FUNCIONES GENERALES    /////////////////////
//////////////////////////////////////////////////////////////////////////////////////

void user_inicializar(void) {
	user_signal_init();
	user_variables_init();
	aux_historia_cargar();
}

void user_finalizar(void) {
	aux_historia_guardar();
    list_delete( &variables );
}

//////////////////////////////////////////////////////////////////////////////////////
// ( G ) ////////////////    FIN DE FUNCIONES  GENERALES    ///////////////////////
//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// [ 1 ] ///////////    INICIO DE FUNCIONES PARA LAS SENYALES    ////////////////
//////////////////////////////////////////////////////////////////////////////////////

 static void user_signal_init(){
    SetConsoleCtrlHandler(NULL,TRUE);
 }

//////////////////////////////////////////////////////////////////////////////////////
// [ 1 ] ////////////    FIN DE FUNCIONES  PARA LAS SENYALES    ///////////////////
//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// [ 2 ] ///////////    INICIO DE LA FUNCION PARA LA ORDEN CD    /////////////////
//////////////////////////////////////////////////////////////////////////////////////

int user_cd(char **cline) {
	/* Implementación */   
    int result = TERMINA_BIEN;
    char *anterior;
	char *aux;
    char *ruta;
    anterior = Get_Current_Directory();
    // cambio al directorio base
    if (cline[1] == NULL) {
		Change_Directory(Get_Env_Variable(SHELL_HOME)) ;
    }
    // cambio al directorio anterior
    else if (!strcmp(cline[1], "-" )) {
		aux =Get_Env_Variable(OLD_DIR);
		if ( aux ){
			Change_Directory(aux);
			printf("%s\n",aux);
			free(aux);
       }
		else {
			Show_Error_Message(SHELL,"cd","No hay un directorio anterior");
			result = TERMINA_MAL;
		}
   }
    else if (!strcmp(cline[1], "+" )) {
		aux = Get_Env_Variable(FST_DIR);
		printf("%s\n",aux);
		Change_Directory(aux);
		free(aux);
	}
    else {
		if ( Change_Directory((cline[1]) ) )
			Show_Error_Message(SHELL,cline[1],"No es un directorio");
   }
    ruta = Get_Current_Directory();
    Set_Env_Variable(CUR_DIR,ruta);
    Set_Env_Variable(OLD_DIR,anterior);
    free(ruta);
	return result;
}


//////////////////////////////////////////////////////////////////////////////////////
// [ 2 ] ////////////    FIN DE LA FUNCION  PARA LA ORDEN CD    ///////////////////
//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// [ 3 ] ////////    INICIO DE LA FUNCION PARA OBTENER EL PROMPT    ////////////
//////////////////////////////////////////////////////////////////////////////////////

char *user_getPrompt(void) {
    /* Implementación */
    char *prompt;
	char drive;
    char *nombre;
    char *ruta;
    char *directorio;
    int promptLeng = 23;
    struct tm *tp;
    time_t t;
  
    time(&t);
    tp = localtime(&t);
    ruta = Get_Current_Directory();
    nombre = Get_User();
    drive = ruta[0];
	directorio = strrchr(ruta,'\\');
	if ( strlen( directorio) > 1 )
		directorio[0] = '>';
	promptLeng += ( strlen(nombre) + strlen(directorio) );
	prompt = (char *) malloc ( sizeof(char) * promptLeng );
	sprintf(prompt,"[%d%d/%d%d/%d%d %d%d:%d%d %s@%c:%s]$",
		( tp->tm_mday/10),    ( tp->tm_mday  % 10),
		((tp->tm_mon + 0)/10),((tp->tm_mon+1)% 10),
		((tp->tm_year/10)%10),( tp->tm_year  % 10),
		( tp->tm_hour/10),    ( tp->tm_hour  % 10),
		( tp->tm_min /10),    ( tp->tm_min   % 10),
		( nombre ),    (drive),      (directorio));
  return prompt;
}

//////////////////////////////////////////////////////////////////////////////////////
// [ 3 ] /////////    FIN DE LA FUNCION PARA OBTENER EL PROMPT    ///////////////
//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// [ 4 ] ///////    INICIO DE FUNCIONES  DEL MECANISMO DE TUBERIAS    ////////
//////////////////////////////////////////////////////////////////////////////////////

static int aux_tuberias_sintaxis(char **comandos){
    int i = 1;
    int numTub = 0;
    if  ( strcmp(comandos[0],PIPE_S) == 0) {
		Show_Error_Message(SHELL," |","Se esperaba un token inicial");
		return 0;
    }
    while (comandos[i]){
		if ( ( ( strcmp(comandos[i],PIPE_S) == 0) ) &&
			( strcmp(comandos[i-1],PIPE_S) == 0) ) {
			Show_Error_Message(SHELL,"|",
							"Se esperaba un token despues del operador");
			return 0;
		}
		else if (strcmp(comandos[i],PIPE_S) == 0)
			numTub++;
		i++;
   }
    if ( strcmp(comandos[i-1],PIPE_S) == 0) {
		Show_Error_Message(SHELL,"|","Se esperaba un token final");
		return 0;
    }
    return numTub;
}

char *aux_tub_orden(char **comandos){
	int i = 1;
	int leng = 2;
	char *aux;
	char *line;
	if (Shell_Comand(comandos[0],OP_INIT) ){
		leng = strlen(comandos[0]) + strlen(SHELL) + strlen("--comand") +3;
		aux = (char *)malloc(sizeof(char)*(leng));
		sprintf(aux,"%s --comand %s",SHELL,comandos[0]);
		comandos[0] = aux;
	}
	while ( comandos[i] )
		leng += strlen(comandos[i++] ) ;
	line = (char *)malloc(sizeof(char)*leng);
	strcpy(line,comandos[0]);
	i = 1;
	while ( comandos[i]){
		strcat(line," ");
		strcat(line,comandos[i++]);
	}
	return line;
}

int user_tuberias(char **comands,int opciones){
    STARTUPINFO *si;
	PROCESS_INFORMATION *pi;
	int tub_par = 1;
	HANDLE pipe_lee, pipe_escribe, pipe_read, pipe_write;
	int tub_entrada = 0, tub_salida = 1;
	int indice = 0;
	int i = 0;
	int numTuberia = 1;
	int maxTub;
	char *comando;
	char **comandos = comands;
	SECURITY_ATTRIBUTES pipe_seguridad =
		{sizeof(SECURITY_ATTRIBUTES),NULL,TRUE};
	SECURITY_ATTRIBUTES pipe_security =
		{sizeof(SECURITY_ATTRIBUTES),NULL,TRUE};
	if ( (maxTub = aux_tuberias_sintaxis(comandos) ) == 0 )
		return TERMINA_MAL;
	si = (STARTUPINFO *) malloc ((maxTub+1)*sizeof(STARTUPINFO));
	pi = (PROCESS_INFORMATION *) malloc ((maxTub + 1)*sizeof(PROCESS_INFORMATION));
	while ( comandos[i] && strcmp(comandos[i],PIPE_S) != 0 ) i++;
	comandos[i] = 0;
	comando =	aux_tub_orden(comandos);
	/////////////////////////////////////////////////////
	// primera tuberia
	CreatePipe(&pipe_lee, &pipe_escribe, &pipe_seguridad, 0);
	GetStartupInfo(&si[0]);
	ZeroMemory(&pi[0],sizeof(pi[0]));
	si[0].hStdInput= GetStdHandle(STD_INPUT_HANDLE);
	si[0].hStdError = GetStdHandle(STD_ERROR_HANDLE);
	si[0].hStdOutput = pipe_escribe;
	si[0].dwFlags = STARTF_USESTDHANDLES;
	if(!CreateProcess(NULL,comando,NULL,NULL,TRUE,
								0, NULL, NULL, &si[0], &pi[0] ) )
		Show_Error_Message(SHELL,comandos[0],"Comando no encontrado");
	CloseHandle(pipe_escribe);
	CloseHandle(pi[0].hThread);
	/////////////////////////////////////////////////////
	comandos += i +1;
	i = 0;
    while ( comandos[i] ) {
		while ( comandos[i] && strcmp(comandos[i],PIPE_S) != 0 )
			i++;
		if ( comandos[i] ) {
			comandos[i] = 0;
			comando = aux_tub_orden(comandos);
			GetStartupInfo(&si[numTuberia]);
			ZeroMemory(&pi[numTuberia],sizeof(pi[numTuberia]));
			if ( tub_par ){
				CreatePipe(&pipe_read, &pipe_write, &pipe_security, 0);			
				si[numTuberia].hStdInput= pipe_lee;
				si[numTuberia].hStdError = GetStdHandle(STD_ERROR_HANDLE);
				si[numTuberia].hStdOutput = pipe_write;
				si[numTuberia].dwFlags = STARTF_USESTDHANDLES;
			}
			else {
				CreatePipe(&pipe_lee, &pipe_escribe, &pipe_security, 0);				
				si[numTuberia].hStdInput= pipe_read;
				si[numTuberia].hStdError = GetStdHandle(STD_ERROR_HANDLE);
				si[numTuberia].hStdOutput = pipe_escribe;
				si[numTuberia].dwFlags = STARTF_USESTDHANDLES;
			}
			if(!CreateProcess(NULL,comando,NULL,NULL,TRUE,
								0, NULL, NULL, &si[numTuberia], &pi[numTuberia] ) )
				Show_Error_Message(SHELL,comandos[0],"Comando no encontrado");
			if ( tub_par ){
				CloseHandle(pipe_write);
				CloseHandle(pipe_lee);
			}
			else{
				CloseHandle(pipe_escribe);
				CloseHandle(pipe_read);
			}
			tub_par = !tub_par;
			tub_entrada = tub_salida;
			tub_salida = !tub_salida;
			numTuberia++;
			comandos += i +1;
			i = 0;
		}
	}
	comando = aux_tub_orden(comandos);
	GetStartupInfo(&si[numTuberia]);
	ZeroMemory(&pi[numTuberia],sizeof(pi[numTuberia]));
	if ( tub_par ) 
		si[numTuberia].hStdInput = pipe_lee; 	
	else
		si[numTuberia].hStdInput = pipe_read;
	si[numTuberia].hStdError = GetStdHandle(STD_ERROR_HANDLE);
	si[numTuberia].hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	si[numTuberia].dwFlags = STARTF_USESTDHANDLES;
	if(!CreateProcess(NULL,comando,NULL,NULL,TRUE,
								0, NULL, NULL, &si[numTuberia], &pi[numTuberia] ) )
	Show_Error_Message(SHELL,comandos[0],"Comando no encontrado");
	if ( tub_par ) 
		CloseHandle(pipe_lee);
	else
		CloseHandle(pipe_read);
	CloseHandle(pi[i].hThread);
	for ( i = 0 ; i <= maxTub ; i++ ) {
		WaitForSingleObject(pi[i].hProcess,INFINITE);
		CloseHandle(pi[i].hProcess);
	}
	return TERMINA_BIEN;
}

//////////////////////////////////////////////////////////////////////////////////////
// [ 4 ] ////////    FIN DE FUNCIONES DEL  MECANISMO DE TUBERIAS    ///////////
//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// [ 5 ] //////    INICIO DE FUNCION PARA EL SOPORTE DE COMODINES    /////////
//////////////////////////////////////////////////////////////////////////////////////

int user_comodines(char *patron,char **destino,int i){
    HANDLE hDirList;
	WIN32_FIND_DATA fdData;
	char *dir = NULL;
	char *aux;
	int k = 0, j = 0;
	int len;
	if ( !strchr(patron,'*') && !strchr(patron,'?') ){
		destino[i++] = strdup(patron);
		return 1;
	}
	while ( patron[j] != '*' && patron[j] != '?' ) j++;
	while ( j && patron[j] != '\\' ) j--;
	if	( patron[j] == '\\' ){
		dir = (char *)malloc(sizeof(char)*(j+2));
		while ( j ){
			dir[k] = patron[k++];
			j--;
		}
		dir[k] = '\\';
		dir[k+1] = 0;
	}
	j = 0;
		if (INVALID_HANDLE_VALUE != (hDirList = FindFirstFile(patron, &fdData))){
			/* Hay al menos un fichero */
			do {
				/* Imprimir el archivo leído */
				if ( fdData.cFileName[0] != '.' || ( patron[0] == '.' ) ) {
					if ( dir ) {
						len = strlen(dir) + strlen(fdData.cFileName) + 1;
						aux =(char *)malloc(sizeof(char)*len);
						sprintf(aux,"%s%s",dir,fdData.cFileName);
						destino[i++] = aux;
					}
					else
						destino[i++] = strdup(fdData.cFileName);
					j++;
				}
			} while (FindNextFile( hDirList, &fdData));
		}
			/* Eliminar memoria y handlers */
			if (hDirList != INVALID_HANDLE_VALUE)
				FindClose(hDirList);
			hDirList = INVALID_HANDLE_VALUE;
   return j;
}

//////////////////////////////////////////////////////////////////////////////////////
// [ 5 ] ////////    FIN DE FUNCION PARA EL SOPORTE DE COMODINES    ///////////
//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// [ 6 ] ///////    INICIO DE FUNCIONES DE LA HISTORIA DE ORDENES    /////////
//////////////////////////////////////////////////////////////////////////////////////

static void aux_historia_cargar(){
	int ind_linea = 0;
    char linea[MAX_LINEA] ;
    FILE *f_winsh_h;
    char c = 0;
    int long_linea;
    char *ruta;
    char *orden_vacia = (char *)malloc(sizeof(char)) ;  
    char *shell_home = Get_Env_Variable(SHELL_HOME);
    
	orden_vacia[0] = '\0';
    list_new(&historia) ;
    long_linea = strlen(shell_home)+18 ;
    ruta = (char *)malloc(sizeof(char)*long_linea) ;
    sprintf( ruta,"%s%s",shell_home,SHELL_HISTORY) ;
    free(shell_home);

    if ((f_winsh_h = fopen(ruta,"r")) != NULL) {
		fscanf(f_winsh_h,"%c",&c) ;
		while( !feof(f_winsh_h) ) {
			ind_linea = 0 ;    
			while( !feof(f_winsh_h) && (c != '\n') && (ind_linea < MAX_LINEA) ) {
				linea[ind_linea] = c ;
				fscanf(f_winsh_h,"%c",&c) ;
				ind_linea++ ;
			}
			if ( ind_linea > 0 ) {  
				linea[ind_linea]='\0';
				list_push_back(&historia,linea) ;
		    }
			fscanf(f_winsh_h,"%c",&c) ;
		}
		fclose(f_winsh_h) ;
	}
    list_push_back(&historia,orden_vacia) ;
    list_it_copy(&posicion,list_last(&historia) ) ;
    free(ruta);
}

static void aux_historia_guardar(){

   FILE *f;
   char *ruta;
   int long_linea;
   char *shell_home =Get_Env_Variable(SHELL_HOME);   
   long_linea = strlen(shell_home)+18 ;
   ruta = (char *)malloc(sizeof(char)*long_linea) ;
   sprintf( ruta,"%s%s",shell_home,SHELL_HISTORY) ;
   free(shell_home);
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
      Show_Warning();
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
	  Show_Warning();
	  free(aux);
	  return NULL;
	}
	free(aux);
    }
  }
  else {
  //  direccion == FLECHA_ABAJO 
    if ( list_it_is_last(&posic) )
		Show_Warning();
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
	  Show_Warning();
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
			list_erase(&historia,orden) ;
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
// [ 6 ] /////////    FIN DE FUNCIONES DE LA HISTORIA DE ORDENES    ///////////
//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// [ 7 ] ////////    INICIO DE FUNCIONES DE LA CARACTERISTICA LS    ////////////
//////////////////////////////////////////////////////////////////////////////////////


static char *aux_ls_l(char *dat,struct _stat buf ){
	char *solucion;
	char *size;
	char fecha[14],fech_ult[6];
	char *permisos;
	int  lengh;
	int  meses, segundos;
	time_t t;
	struct tm *tp;
	permisos = Get_File_Attributes(dat);
	time(&t);
	tp = localtime(&t);
	meses = 12 * tp->tm_year + tp->tm_mon;
	segundos = t;
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
	lengh = strlen(permisos) + strlen(size)  +  strlen(fecha)  +  strlen(dat)  +  17;
	solucion = (char *)malloc(sizeof(char)*(lengh));
	sprintf(solucion ,"%s %s %s %s",permisos,size,fecha,dat);
	return solucion;
}

static int aux_ls_mostrar(list_t *lista,int opc_l, int son_fich){
	char **ficheros,**aux;
	int  size, i;
	size = list_size(lista);
	if ( size < 1 )
	   return 0;
	ficheros = list_to_array(lista);
	if ( opc_l )
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

static int aux_ls_directorio(int opc,int endl){
	HANDLE hDirList;
	WIN32_FIND_DATA fdData;
    char *direc = ".\\*";
	char *datos ;
    int  opc_a ;
    int  opc_l ;
    char  *nombre_fich ;
    struct _stat buf ;
    list_t  lista_fich ;
    opc_a = opc % 2;
    opc_l = opc / 2;
   	if (INVALID_HANDLE_VALUE != 
	   (hDirList = FindFirstFile(direc, &fdData))) {
        list_new(&lista_fich) ;
		do{
		// opcion -a
			if (fdData.cFileName[0] != '.' || opc_a ) {
				nombre_fich = strdup(fdData.cFileName);
				if (_stat(nombre_fich,&buf) != -1) {
					datos = strdup(fdData.cFileName);
					// opcion -l
					if ( opc_l )
					datos = aux_ls_l(datos,buf);
					list_push_back(&lista_fich,datos);
					free(datos);
				}
			}
		} while (FindNextFile( hDirList, &fdData));
		FindClose(hDirList);
		if ( list_size(&lista_fich) == 0 ){
			list_delete(&lista_fich);
			return 0;
        }
        if (endl) printf("\n");
		aux_ls_mostrar(&lista_fich,opc_l,0);
		list_delete(&lista_fich);
		return 1;
    }
			/* Eliminar memoria y handlers */
			if (hDirList != INVALID_HANDLE_VALUE)
				FindClose(hDirList);
			hDirList = INVALID_HANDLE_VALUE;
			return 1;
	return 1;
}

static int aux_ls_ficheros(char **comandos,int opc,int size){
    int i = 1, j = 1, muestra = 0;
    int opc_a, opc_l ;
    char *datos;
    char **comand =(char**)malloc(sizeof(char*)*(size+1));
    struct _stat buf ;
    list_t lista_fich ;
    opc_a = opc % 2;
    opc_l = opc / 2;
    list_new( &lista_fich );
    while ( comandos[i] ) {
		if ( comandos[i][0] == '-'){
			if (comandos[i][1] == '-' ){
				printf("\aERROR: ls: '%s': ",comandos[i]);
				printf("Opcion invalida \n");
				printf("opciones validas: [-a] [-l]\n");
				return -1;
			}
        }
        else if (_stat(comandos[i],&buf) != -1) {
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
						list_push_back(&lista_fich,datos);
						free(datos);
				}
				free(comandos[i]);
			}
        }
		else {
			Show_Error_Message(SHELL,comandos[i],"No Existe el fichero o directorio");
		}
		i++;
    }
	comand[j] = NULL;
    muestra = (  list_size(&lista_fich) > 0 );
    if ( muestra )
		aux_ls_mostrar(&lista_fich,opc_l,1);
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

int user_ls(char ** comandos){
	int  i = 0;
	int opcion;
	int result;
	int print = 0;
	int opc_l = 0;
	int opc_a = 0;
	int num_parametros;
	char opc[MAX_OPCIONES];
	char *dir = Get_Current_Directory();
	// devuelve el numero de argumentos que no son una opcion
	num_parametros = opciones_con_menos(comandos,opc);
	while ( opc[i] ){
	   if ( opc[i] == 'a' )
	      opc_a = 1;
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
	opcion = 2*opc_l + opc_a;
	if ( num_parametros == 1 ){
	    result = aux_ls_directorio(opcion,0);
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
		Change_Directory(comandos[i] );
	    if ( num_parametros > 2 ) {
	        if ( print )
	            printf("\n");
			printf("%s:",comandos[i]);
	    }
	    result = aux_ls_directorio(opcion,(num_parametros > 2 ));
	    Change_Directory(dir);
	    if ( result > 0 )
			print = 1;  
	    i++;
	}
	return TERMINA_BIEN;
}

//////////////////////////////////////////////////////////////////////////////////////
// [ 7 ] //////////    FIN DE FUNCIONES DE LA CARACTERISTICA LS    //////////////
//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// [ 8 ] //    INICIO DE FUNCIONES PARA LOS SEPARADORES AND Y OR    //////////
//////////////////////////////////////////////////////////////////////////////////////


int aux_or_and_sintaxis(char **comandos){
  int i = 0;
  if ( ( strcmp(comandos[0],AND_S) == 0) ||
       ( strcmp(comandos[0], OR_S) == 0)  ) {
	Show_Error_Message(SHELL, ( 1 + comandos[0] ),
										"Se esperaba un token inicial");
	return 0;
   }
   while (comandos[i]){
      if ( ( ( strcmp(comandos[i],AND_S) == 0) ||
	     ( strcmp(comandos[i], OR_S) == 0) ) &&
	   ( ( strcmp(comandos[i-1],AND_S) == 0) ||
	     ( strcmp(comandos[i-1], OR_S) == 0) ) ){
	  Show_Error_Message(SHELL, ( 1 + comandos[0] ),
											"Se esperaba un token despues del operador");
	  return 0;     
      }
      i++;
   }
   if ( ( strcmp(comandos[i-1],AND_S) == 0) ||
	     ( strcmp(comandos[i-1], OR_S) == 0)  ) {
       Show_Error_Message(SHELL, ( 1 + comandos[0] ),
										  "Se esperaba un token final");
      return 0;
   }
  return 1;
}

int user_or_and(char **comandos){
    int   terminar = 0;
    int   resultado;
    int   j, i = 0;
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
        if ( Shell_Comand(comand[0],OP_INIT) )
			resultado = Shell_Exec(comand,OP_INIT,-1);
		else
			resultado = System_Exec( comand,FOREGROUND, -1);
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
// [ 8 ] ///    FIN DE FUNCIONES PARA LOS SEPARADORES AND Y OR    /////////////
//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// [ 9 ] //////    INICIO DE FUNCIONES DE LA  CARACTERISTICA ARROBA    ////////
//////////////////////////////////////////////////////////////////////////////////////

int user_arroba_exec(int num,char**comandos, int time) {
	int i = 0, j = 0;
	int opc;
	int lenght;
	char *auxiliar;
	char *st_comand;
	char **c = comandos + 2 ;
	opc = Shell_Operators(comandos+1,num-1);
	st_comand = First_Word_Comand(num-1,comandos+1);
	if ( st_comand ) {
		while ( st_comand != comandos[j] ) j++;
		if ( Shell_Comand(st_comand,opc) ){
			lenght = strlen(SHELL) + strlen("--comand") + 2 ;
			auxiliar = (char *)malloc(sizeof(char)*lenght);
			sprintf(auxiliar,"%s --comand",SHELL);
			j--;
			comandos[j] = auxiliar;
		}
		return System_Exec(comandos+j, FOREGROUND,time);
	}
	Show_Error_Message(SHELL,"--arroba","Necesita almenos un comando");
	return TERMINA_MAL;
}

int user_arroba(char **comandos,int time){
	char *auxiliar;
	int lenght = strlen(SHELL) + strlen(comandos[0]) + 16;
	auxiliar = (char *)malloc(sizeof(char)*lenght);
	sprintf(auxiliar,"%s --A%d %s",SHELL,time,comandos[0]);
	comandos[0] = auxiliar;
	printf("\n");	
	return System_Exec(comandos,BACKGROUND,-1);
}

//////////////////////////////////////////////////////////////////////////////////////
// [ 9 ] ////////    FIN DE FUNCIONES DE LA CARACTERISTICA ARROBA    ///////////
//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// [ 10 ] //////    INICIO DE FUNCIONES PARA EL MANEJO DE VARIABLES    ////////
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

int user_variables_es_definicion(char *entrada){
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
				valor = Get_Env_Variable(auxiliar[1]);
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

static void user_variables_init(){
	char *homePath = Get_Env_Variable("HOMEPATH");
	char *homeDrive = Get_Env_Variable("HOMEDRIVE");
    char *shell_home;
	int lenght = strlen(homePath) + strlen(homeDrive) + 1;
	list_new ( &variables );
	shell_home = (char *)malloc(sizeof(char)*lenght);
	sprintf(shell_home,"%s%s",homeDrive,homePath);
	Set_Env_Variable(SHELL_HOME,shell_home);
	Set_Env_Variable(CUR_DIR,Get_Current_Directory());
	Set_Env_Variable(FST_DIR,Get_Current_Directory());
	Set_Env_Variable(NAME_V,VERSION);
	free(homePath);
	free(homeDrive);
	free(shell_home);
}

int user_definicion(char **def){
	/* Implementacion */
	char *definicion;
	char *nombre;
	char *valor;
	definicion = strdup(def[0]);
	nombre = strtok(definicion, "=");
	valor  = strtok(   NULL   ,"\0");
	// para evitar tener variables duplicadas
	if  (   ( aux_variables_get(nombre) == NULL )&&
		( Get_Env_Variable(nombre) != NULL )    )
		Set_Env_Variable(nombre,valor);
	else {
	    aux_variables_erase(nombre);
	    list_insert(&variables,def[0]);
	}
	return TERMINA_BIEN;
}

int  user_set(){
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
		return TERMINA_BIEN;
}

int  user_unset(char **variables){
	int i = 1;
	char *aux;
	while ( variables[i] != NULL ) {
	    if (aux_variables_erase(variables[i]));
	    else if ( ( aux =Get_Env_Variable(variables[i]) )!= NULL){
			free(aux);
			if ( strncmp(variables[i],INIT_VAR_SHELL,strlen(INIT_VAR_SHELL)) )
				Unset_Env_Variable(variables[i]);
			else
				Show_Error_Message(SHELL,variables[i],"No se puede eliminar");
		}
	    else {
			Show_Error_Message(SHELL,variables[i],"No existe");
			return TERMINA_MAL;
	    }
	    i++;
	}
	if ( i == 1 ){
		Show_Error_Message(SHELL,"unset",
			"Necesita almenos el una variable");
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
	      Set_Env_Variable(variables[i],valor);
	      aux_variables_erase(variables[i]);
	   }
	   else{
	      Show_Error_Message(SHELL,variables[i],
											 "No es una variable local");
	      return TERMINA_MAL;
	   }
	   i++;
	}
	if ( i == 1 ){
	  Show_Error_Message(SHELL,"export",
			"Necesita almenos una variable local");
	   return TERMINA_MAL;
	}
	else
	   return TERMINA_BIEN;
}

int user_env(){
	char *var_entorno = GetEnvironmentStrings();
	while (*var_entorno) {
		while (*var_entorno)
			putchar(* var_entorno++); 
		putchar('\n');
		var_entorno++;
	}
	FreeEnvironmentStrings(var_entorno);
   return TERMINA_BIEN;
}

//////////////////////////////////////////////////////////////////////////////////////
// [ 10 ] ///////    FIN DE FUNCIONES PARA  EL MANEJO DE VARIABLES    //////////
//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// [ 11 ] ///////////////    INICIO DE  FUNCION ORDEN EXIT    /////////////////////
//////////////////////////////////////////////////////////////////////////////////////

int user_exit(){
	comand_exit();
    return TERMINA_BIEN;
}

//////////////////////////////////////////////////////////////////////////////////////
// [ 11 ] ////////////////    FIN DE  FUNCION ORDEN EXIT    ////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// < MEJORA 1 > ///   INICIO DE FUNCIONES  PARA EL USO DEL TABULADOR   //////
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
       if ( strncmp(shell_order[i],patron,leng) == 0 ){
	  list_insert(l,shell_order[i]);
       }
       i++;
    }
}

int aux_tab_segunda_vez(list_t *soluciones){
    char c;
    char **t_aux, **t;
    int i, j;
    int valida;
    int mostrar = 1;
    int lenght = list_size(soluciones);
    HANDLE entrada = GetStdHandle(STD_INPUT_HANDLE);
    if ( lenght > 50 ){
		valida = 0;
		printf("\n");
		while ( ! valida ) {
			printf("\rHay %d ",lenght);
			printf("posibilidades, ¿ desea continuar ? ( s / n )");
			c = LeerTecla(entrada);
			if ( c == 's' || c == 'n'){
				valida = 1;
			    mostrar = ( c == 's' ? 1 : 0 );
			}
			else
			    Show_Warning();
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
				c = LeerTecla(entrada);
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
					default  : Show_Warning(); break;
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

static void aux_tabulador_dir(char *patron,list_t *l,int opc){
    HANDLE hDirList;
	WIN32_FIND_DATA fdData;
	int len = strlen(patron)+4;
	char *dir;
	int es_dir;
	int es_eje;
	dir = (char *)malloc(sizeof(char)*len);
	sprintf(dir,".\\%s*",patron);
	len -= 4;
		if (INVALID_HANDLE_VALUE != (hDirList = FindFirstFile(dir, &fdData))){
			/* Hay al menos un fichero */
			do {
				if ( ( strcmp (fdData.cFileName,"." ) != 0 ) && 
					( strcmp (fdData.cFileName,"..") != 0 ) && 
					( strncmp(fdData.cFileName,patron,len) == 0 ) ) { 
					es_dir = 0;
					es_eje = 0;
					if (fdData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
						es_dir = 1;
					else if  ( strsub(fdData.cFileName,".") &&
								(  ( strcmp(strrchr(fdData.cFileName,'.'),".exe") == 0 ) ||
								( strcmp(strrchr(fdData.cFileName,'.'),".com") == 0 )  ) )
						es_eje = 1;
					if ( es_dir || es_eje || opc ){
						list_erase(l,fdData.cFileName);
						list_insert(l,fdData.cFileName);
					}
				}
			} while (FindNextFile( hDirList, &fdData));
		}
			/* Eliminar memoria y handlers */
			if (hDirList != INVALID_HANDLE_VALUE)
				FindClose(hDirList);
			hDirList = INVALID_HANDLE_VALUE;
}


char *user_tabulador(char * parte_recibida, int numero, int numtab) {
	list_t soluciones;
	int  leng;
	char *rutas = NULL;
	char *directorio = NULL;
	char *var;
	char *aux;
	char *ruta, *auxiliar;
	char *parte = strdup(parte_recibida);
	char *parte_final,  *solucion;
	char *dir = Get_Current_Directory();
	list_new(&soluciones);
	if ( (var = aux_tab_es_variable(strdup(parte_recibida))) != NULL ){
		parte_final = strdup(var);
	    ruta = strdup("");
	    aux_tab_variables(&soluciones,var);
	}
    else {
	    if ( ( aux = user_variables(parte) ) != NULL )
			parte = aux;
	    if( !strsub(parte,"\\") ) {
			parte_final = strdup(parte);
			if( !strsub(parte,"\\") ) {
				parte_final = strdup(parte);
				ruta = strdup(".");
				if( numero == 1)
					rutas = strdup(Get_Env_Variable("Path"));
			}
	    }
	    else {
			parte_final = strdup(strrchr(parte_recibida,'\\')+1);
			ruta = strdup(parte_recibida);
			strrchr(ruta,'\\')[0] = 0;
			//printf("\n'%s'\n'%s'\n",parte_final,ruta);
	    }
		if ( Change_Directory(ruta) == 0 ) {
			aux_tabulador_dir(parte_final,&soluciones,(numero>=2));
			Change_Directory(dir);
		}
		if ( rutas )
			directorio = strtok(rutas,";");
	    while ( directorio != NULL ) {
			if ( Change_Directory(directorio) == 0 ) { 
				aux_tabulador_dir(parte_final,&soluciones,(numero>=2));
				Change_Directory(dir);
			}
			directorio = strtok( NULL ,";" );
		}
	    if ( numero ==  1 && !strsub(parte,"\\") )
			aux_tab_comandos_internos(&soluciones,parte_final);
	}
	if ( list_size(&soluciones) == 0){
	    Show_Warning();
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
			Show_Warning();
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
			Show_Warning();
			free(ruta);
			free(parte_final);
			return NULL;
	    }
	    solucion = strdup(solucion+leng);
	    free(ruta);
	    free(parte_final);
	    return solucion;
	}
}


//////////////////////////////////////////////////////////////////////////////////////
// < MEJORA 1 > ///   FIN DE FUNCIONES  PARA EL USO DEL TABULADOR   //////////
//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// < MEJORA 2 > /////////   INICIO DE FUNCIONES TAIL Y HEAP   //////////////////
//////////////////////////////////////////////////////////////////////////////////////

int aux_tail_head_lineas(char **comandos,int *opc_n){
	int solucion = 10;
	*opc_n = 0;
	if ( comandos[1][0] == '-' && comandos[1][1] ){
		if ( strcmp(comandos[1],"-n") == 0 ){
			if ( strton(comandos[2],&solucion) ){
				*opc_n = 1;
				return solucion;
			}
			else{
				Show_Error_Message(SHELL,comandos[2],"Opcion erronea");
				return 0;
			}
		}
		else if ( strton(comandos[1]+1,&solucion) )
			return solucion;
		else{
			Show_Error_Message(SHELL,comandos[1],"Opcion erronea");
			return 0;
		}
	}
	else
		return solucion;
}

int  user_tail(char **comandos){
	HANDLE informacion;
	HANDLE mapeo;
	int lineas;
	int c;
	int opc_n;
	int cont;
	char *aux_p_fichero;
	char *p_fichero;
	char *fichero;
	if ( ( lineas = aux_tail_head_lineas(comandos,&opc_n) ) > 0 ){
		if ( ! lineas )
			return TERMINA_BIEN;
		if ( !opc_n )
			fichero = First_Word_Comand( LAST_IS_NULL,comandos+1);
		else
			fichero = First_Word_Comand( LAST_IS_NULL,comandos+3);
		if ( fichero ) {
			informacion = CreateFile(fichero,	GENERIC_READ,0,	NULL,
									OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
			if (informacion==INVALID_HANDLE_VALUE) {
				Show_Error_Message(SHELL,fichero,"Fichero invalido");
				return TERMINA_MAL;
			}
			else {
				mapeo=CreateFileMapping(informacion,NULL,PAGE_READONLY,
															0,	0,	"MyFileMappingObject");
				if (mapeo != NULL) {
					p_fichero=MapViewOfFile(mapeo,FILE_MAP_READ,0,0,0);
					if ( p_fichero != NULL){
						aux_p_fichero=p_fichero;
						cont =0;
						while (*p_fichero!= 0)  {
							p_fichero++;
							if (*p_fichero == '\n') cont++;
						}
						if (cont < lineas) {
							printf("%s \n",aux_p_fichero);
						}
						else {
							cont=0;
							c = *p_fichero-1;
							while ( ( cont < lineas ) && 
										( p_fichero != 0) ){
								if ( *p_fichero == '\n' )
									cont++;
								* p_fichero--;
							}
							*p_fichero++;
							if ( *p_fichero != 0 )
								*p_fichero++;
      						printf("%s\n",p_fichero);
						}
						CloseHandle(mapeo);
						CloseHandle(informacion);
						UnmapViewOfFile(p_fichero);
						return TERMINA_BIEN;
					}
					else{
						Show_Error_Message(SHELL,"tail","Could not map view of file");
						return TERMINA_MAL;
					}
				}
				else {
					Show_Error_Message(SHELL,"tail","Could not create file mapping object");
					return TERMINA_MAL;
				}
			}
		}
		else
			return TERMINA_MAL;
	}
	return TERMINA_MAL;
}

int  user_head(char **comandos){
	HANDLE informacion;
	HANDLE mapeo;
	int lineas;
	int opc_n;
	int cont;
	char *p_fichero;
	char *fichero;
	if ( ( lineas = aux_tail_head_lineas(comandos,&opc_n) ) > 0 ){
		if ( ! lineas )
			return TERMINA_BIEN;
		if ( !opc_n )
			fichero = First_Word_Comand( LAST_IS_NULL,comandos+1);
		else
			fichero = First_Word_Comand( LAST_IS_NULL,comandos+3);
		if ( fichero ) {
			informacion = CreateFile(fichero,	GENERIC_READ,0,	NULL,
									OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
			if (informacion==INVALID_HANDLE_VALUE) {
				Show_Error_Message(SHELL,fichero,"Fichero invalido");
				return TERMINA_MAL;
			}
			else {
				mapeo=CreateFileMapping(informacion,NULL,PAGE_READONLY,
															0,	0,	"MyFileMappingObject");
				if (mapeo != NULL) {
					p_fichero=MapViewOfFile(mapeo,FILE_MAP_READ,0,0,0);
					if ( p_fichero != NULL){
						cont =0;
						while (cont < lineas) {
							while ( (*p_fichero!='\n') && 
										(*p_fichero!='\0') )
								putchar(* p_fichero++);
							putchar('\n');
							if (*p_fichero=='\0') break;
							p_fichero++;
							cont++;
						}
						CloseHandle(mapeo);
						CloseHandle(informacion);
						UnmapViewOfFile(p_fichero);
						return TERMINA_BIEN;
					}
					else{
						Show_Error_Message(SHELL,"tail","Could not map view of file");
						return TERMINA_MAL;
					}
				}
				else {
					Show_Error_Message(SHELL,"tail","Could not create file mapping object");
					return TERMINA_MAL;
				}
			}
		}
		else
			return TERMINA_MAL;
	}
	return TERMINA_MAL;
}

//////////////////////////////////////////////////////////////////////////////////////
// < MEJORA 2 > //////////   FIN DE FUNCIONES TAIL Y HEAP   /////////////////////
//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// < MEJORA 3 > //////////////   INICIO DE  FUNCIONES   //////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

int user_clear(){
    system("cls");
	return TERMINA_BIEN;
}

int user_comand_exec(int number,char **comand){
	int opt = Shell_Operators(comand,number);
	// por aqui solo entra si se ejecuta winsh ... sin la opcion --arroba'n'
	return runcommand(comand+1,BACKGROUND,opt,-1);
}

int user_echo(char **comand){
	int i = 1;
	while ( comand[i] )
		printf("%s ",comand[i++]);
	printf("\n");
	return TERMINA_BIEN;
}

int user_see(){
	char *aux;
	int i = 0;
	int j;
	user_clear();
	printf("\n\n");
	printf("\tFECHA : \t%s\n\n",SHELL_FECHA);
	printf("\tVERSION :\n\t\t\t%s\n",VERSION);
	printf("\tAUTORES :\n\t\t\t%s\n\t\t\t%s\n",AUTOR1,AUTOR2);
	printf("\tDIRECTORIOS :\n");
	aux = Get_Env_Variable(SHELL_HOME);
	printf("\t\tHOME :                %s\n",aux);
	free(aux);
	printf("\t\tHISTORIA DE ORDENES : $%s%s\n",SHELL_HOME,SHELL_HISTORY);
	printf("\t\tACTUAL :              $%s\n",CUR_DIR);
	printf("\t\tINICIAL :             $%s\n",FST_DIR);
	printf("\t\tANTERIOR :            $%s\n\n",OLD_DIR);
	printf("\tCOMANDOS :\n\t");
	while ( i < COMANDOS_INT ){
		for (j = 0 ; j < 4 ; j++ ){
			if ( i < COMANDOS_INT )
				printf("\t%s",shell_order[i]);
			i++;
		}
		printf("\n\t");
	}
	printf("\n\n");
	return TERMINA_BIEN;
}

//////////////////////////////////////////////////////////////////////////////////////
// < MEJORA 3 > ////////////////   FIN DE  FUNCIONES   ////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

/* $Id: userfn.c 922 2005-01-19 23:34:54Z dsevilla $ */
