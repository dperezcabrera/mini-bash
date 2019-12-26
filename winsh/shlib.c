#ifndef SHLIB_C
#define SHLIB_C

#include "list.h"
#include "shlib.h"
#include "winsh.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

void Show_Error_Message(char *shell, char *comando, char *mensaje){
	printf("%s: %s: %s: %s\n",ERROR_MESSAGE,shell,comando,mensaje);
}

void Show_Warning(){
	printf("%s",WARNING);
}

void Delete_Array(int size, char **t){
	int i = 0;
	if ( t == NULL )
		return;
	while( ( i < size ) || 
				(	( size == LAST_IS_NULL ) &&
					( t[i] != NULL )	)	)
		free(t[i++]);
	free(t);
}

char **Word_Options(int number,  char **comand){
	char **result;
	int i = 0;
	int size;
	int quit = 0;
	list_t lista;
	list_new(&lista);
	while( (  (  i < number ) || 
				(  ( number == LAST_IS_NULL ) && 
					( comand[i] != NULL ) ) ) &&
				!quit ) {
		if ( ( comand[i][0] == '-' ) && 
		      ( comand[i][1] == '-' )  ){
			list_push_back(&lista,comand[i]);
		}
		else if ( comand[i][0]  != '-' )
			quit = 1;
		i++;
	}
	size = list_size(&lista);
	if ( size == 0 ) {
		list_delete(&lista);
		return NULL;
	}
	else {
		result = (char **)malloc(sizeof(char *)*(size+1));
		i = 0;
		while ( list_size(&lista) > 0 ) {
			result[i++] = strdup(list_front(&lista));
			list_pop_front(&lista);
		}
		result[i] = NULL;
		list_delete(&lista);
		return result; 
	}
}

char *Char_Options(int number,  char **comand){
	char *result;
	char op[2];
	int i = 0;
	int j;
	int size;
	int quit = 0;
	list_t lista;
	op[1] = 0;
	list_new(&lista);
	while( (	(  i < number ) || 
				(  ( number ==  LAST_IS_NULL ) && 
					( comand[i] != NULL ) ) ) &&
				!quit ) {
		if ( ( comand[i][0] == '-' ) && 
		      ( comand[i][1] != '-' )  ) {
			j = 1;
			while ( isalpha(comand[i][j]) ){
				op[0] = comand[i][j];
				list_push_back(&lista,op);
				j++;
			}
		}
		else if ( comand[i][0]  != '-' )
			quit = 1;
		i++;
	}
	size = list_size(&lista);
	if ( size == 0 ) {
		list_delete(&lista);
		return NULL;
	}
	else {
		result = (char *)malloc(sizeof(char)*(size+1) );
		i = 0;
		while ( list_size(&lista) > 0 ) {
			result[i++] = list_front(&lista)[0];
			list_pop_front(&lista);
		}
		result[i] = 0;
		list_delete(&lista);
		return result; 
	}
}

static int is_number(char *c){
	int i = 0;
	while (c[i] && isdigit(c[i++] ) ) ;
	return ( c[i] == 0 );
}

char **Number_Options(int number,  char **comand){
	char **result;
	int i = 0;
	int size;
	int quit = 0;
	list_t lista;
	list_new(&lista);
	while( (  (  i < number ) || 
				(  ( number == LAST_IS_NULL ) && 
					( comand[i] != NULL ) ) ) &&
				!quit ) {
		if ( ( comand[i][0] == '-' ) && 
		      ( comand[i][1] != '-' )  &&
			  ( is_number(comand[i] + 1) ) ){
			  list_push_back(&lista,comand[i]);
		}
		else if ( comand[i][0]  != '-' )
			quit = 1;
		i++;
	}
	size = list_size(&lista);
	if ( size == 0 ) {
		list_delete(&lista);
		return NULL;
	}
	else {
		result = (char **)malloc(sizeof(char *)*(size+1));
		i = 0;
		while ( list_size(&lista) > 0 ) {
			result[i++] = strdup(list_front(&lista));
			list_pop_front(&lista);
		}
		result[i] = NULL;
		list_delete(&lista);
		return result; 
	}
}

char *First_Word_Comand(int number,char **comand){
	int i = 0;
	while   (  (  i < number ) || 
				(  ( number == LAST_IS_NULL ) && 
					( comand[i] != NULL ) ) ) {
		if ( comand[i][0] != '-' )
			return comand[i];
		i++;
	}
	return NULL;
}

#endif