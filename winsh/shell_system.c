#ifndef SHELL_SYSTEM_C
#define SHELL_SYSTEM_C

#include <windows.h>
#include "shell_system.h"


char *Get_Env_Variable(const char *nombre){
	char a[2];
	char *valor;
	int longitud;
	longitud = GetEnvironmentVariable(nombre,a,1) +1;
	if ( longitud == 1 )
		return NULL;
	valor = (char *)malloc(sizeof(char)*longitud);
	GetEnvironmentVariable(nombre,valor,longitud-1);
	return valor;
}

int Set_Env_Variable(const char *nombre,const char * valor){
	return SetEnvironmentVariable(nombre,valor);
}

int Unset_Env_Variable(const char *nombre){
	return SetEnvironmentVariable(nombre,NULL);
}

char * Get_User(){
	DWORD sizebuf = 1024;
	LPTSTR nombre;
	nombre = (LPTSTR) malloc(sizebuf*sizeof(char));
	GetUserName(nombre,&sizebuf);
	return nombre;
}

char * Get_Current_Directory(){
	char *ruta;
	DWORD sizebuf = 1024;
	ruta = (char *) malloc(sizebuf*sizeof(char));
	GetCurrentDirectory(sizebuf,ruta);
	return ruta;
}

int Change_Directory(char* new_dir){
	return  (SetCurrentDirectory(new_dir) == 0);
}

char *Get_File_Attributes(char *name){
	char *aux;
	char *x;
	int i = 0;
	int opc = GetFileAttributes(name);
	if ( opc ==  0xFFFFFFFF )
		return NULL;
	aux = (char *)malloc(sizeof(char)*15);
	while ( i < 11 ) aux[i++] = '-';
	aux[11] = ' ';
	aux[12] = ' ';
	aux[13] = 0;
	if (opc & FILE_ATTRIBUTE_HIDDEN )
		aux[0] = 'H';
	if (opc & FILE_ATTRIBUTE_SYSTEM )
		aux[1] = 'S';
	if (opc & FILE_ATTRIBUTE_NORMAL )
		aux[2] = 'N';
	if ( opc & FILE_ATTRIBUTE_COMPRESSED )
		aux[3] = 'C';
	if ( opc & FILE_ATTRIBUTE_ARCHIVE )
		aux[4] = 'A';
	if (opc & FILE_ATTRIBUTE_ENCRYPTED )
		aux[5] = 'E';
	if (opc & FILE_ATTRIBUTE_READONLY )
		aux[6] = 'R';
	if ( opc & FILE_ATTRIBUTE_DIRECTORY ) {
		aux[7] = ' ';
		aux[8] = '<';
		aux[9] = 'D';
		aux[10] = 'I';
		aux[11] = 'R';
		aux[12] = '>';
	}
	else{
		if (opc & FILE_ATTRIBUTE_OFFLINE )
			aux[7] = 'O';
		if (opc & FILE_ATTRIBUTE_REPARSE_POINT )
			aux[8] = 'R';
		if (opc &FILE_ATTRIBUTE_SPARSE_FILE )
			aux[9] = 'S';
		if (opc & FILE_ATTRIBUTE_TEMPORARY )
			aux[10] = 'T';
		if ( ( x = strrchr(name,'.') ) != NULL ) {
			if  (  (strcmp(strrchr(name,'.'),".exe") == 0 ) ||
				( strcmp(strrchr(name,'.'),".com") == 0 )  )
				aux[11] ='X';
		}
	}
	return aux;
}

#endif