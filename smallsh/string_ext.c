#ifndef STRING_EXT_C
#define STRING_EXT_C

#include "string_ext.h"

#include  <ctype.h>
#include  <stdio.h>
#include <stdlib.h>
#include <string.h>
#include   <time.h>

int strsub(char *cadena,char * subCadena){
   int i = 0;
   int j = 0;
   while ( cadena[i] && subCadena[j] ){
      if ( cadena[i] -  subCadena[j] ){
        i -= j;
      	j = 0;
      }
      else
        j++;
      i++;
   }
   return (!subCadena[j]);
}

char *strbeq(char *c1,char *c2){
	char *solucion;
	int i = 0;
	if ( !c1 || !c2 )
	   return strdup(""); 
	solucion = strdup(c1);
	while (  solucion[i] && c2[i] &&
	      !( solucion[i] -  c2[i] ) )
		i++;
	solucion[i] = 0;
	return solucion;
}

char *strnsc(char *s,char c){
   int i = 0;
   if ( !s )
      return NULL;
   while  ( s[i] && ( s[i] - c ) ) i++;
   return ( s[i] -  c ) ? ( NULL ) : ( s + i );
}

char *strnss(char *s,char *c){
   int i = -1, j = 0;
   if ( !s || !c )
      return NULL;
   do {
      i++;
      j = 0;
      while ( s[i] && c[j] && ( s[i] - c[j] ) )j++;
   }while ( s[i] && ( s[i] - c[j] ) );
   return ( s[i] != 0 ) ? ( s + i ) : ( NULL ); 
}

int strton(char *s,int *pnumero){
    int i = 0;
    if ( !s || !pnumero )
      return 0;
    else{
       while ((s[i] <= '9') && (s[i] >= '0')) i++;
       if (!s[i]){
          (*pnumero) = atoi(s);
	  return 1;
       }
       else
       	return 0;
    }
}

char *strbri(int entero,int size,char relleno){
	int i;
	char reyeno[2];
	char *solucion = (char *)malloc(sizeof(char)*(size+1));
	char *integer = (char *)malloc(sizeof(char)*(size+1));
	sprintf(integer,"%d",entero);
	strcpy(solucion,"");
	i = strlen(integer);
	if ( i > size ){
	   free(solucion);
	   return integer;
	}
	reyeno[0] = relleno;
	reyeno[1] = 0;
 	while( i < size ){ i++; strcat(solucion,reyeno);}
	strcat(solucion,integer);
	free(integer);
	return solucion;
}

char *strbrs(char *c,int size,char relleno){
	int i;
	char reyeno[2];
	char *cadena;
	char *solucion = (char *)malloc(sizeof(char)*(size+1));
	strcpy(solucion,"");
	if ( c )
	   cadena = strdup(c);
	else
	   cadena = strdup("");
	i = strlen(cadena);
	if ( i > size )
	   return c;
	reyeno[0] = relleno;
	reyeno[1] = 0;
 	while( i < size ){ i++; strcat(solucion,reyeno);}
	strcat(solucion,cadena);
	free(cadena);
	return solucion;
}

char *strers(char *c,int size,char relleno){
	int i;
	char reyeno[2];
	char *cadena;
	char *solucion;
 	if ( c )
	   cadena = strdup(c);
	else
	   cadena = strdup("");
	i = strlen(cadena);
	if ( i >= size ){
	   return cadena;
	}
	solucion = (char *)malloc(sizeof(char)*(size+1));
	reyeno[0] = relleno;
	reyeno[1] = 0;
	strcpy(solucion,cadena);
 	while( i < size ){ i++; strcat(solucion,reyeno);}
	free(cadena);
	return solucion;
}

char *strtup(char *c){
	int i = 0;
	if ( c ){
	   while ( c[i] ){
	      c[i] = toupper(c[i]);
	      i++;
	   }
	}
	return c;   
}

char *strtlo(char *c){
	int i = 0;
	if ( c ){
	   while ( c[i] ){
	      c[i] = tolower(c[i]);
	      i++;
	   }
	}
	return c;   
}

static int auxqs_particion(char **t,int ini,int fin,cmp_string_ext comp){
   char *temp,*pivote;
   int i = ini + 1,d = fin, aux;
   // suponemos que hay secuencias ordenadas
   srand(time(NULL));
   aux = ini + rand()%(fin - ini);
   pivote = t[aux];
   t[aux] = t[ini];
   t[ini] = pivote;
   while (( i < d ) && (comp(pivote,t[i]) > -1)) i++;
   while ( comp(pivote,t[d]) < 1 ) d--;
   while ( i < d ) {
      temp = t[i];
      t[i] = t[d];
      t[d] = temp;
      while (( i < d ) && (comp(pivote,t[i]) > -1)) i++;
      while ( comp(pivote,t[d]) < 1 ) d--;
   }
   t[ini] = t[d];
   t[d] = pivote;
   return d;
}

static void auxss(char **t,int n,cmp_string_ext c){
   int salto,i,j;
   char *p;
   // Algoritmo de ordenacion por saltos ( ordenacion Shell ).
   for(salto = n/2 ; 0 < salto ; salto /= 2 ) {
     for(i = salto ; i < n ; i++ ) {
       for(j = i - salto ; 0 <= j ; j -= salto ) {
	  // Comprobamos si hay que intercambiar.
	  if ( c(t[j+salto],t[j]) < 0 ) {
	  // Intercambio.
	     p = t[j+salto] ;
	     t[j+salto] = t[j] ;
	     t[j] = p ;
	  }
       }
     }
   }
}

static int auxqs(char **t,int ini, int fin,cmp_string_ext c){
  int j = -1;
  if ( ( fin  - ini ) < 100 ){
     if ( ( fin - ini ) > 0 ){
        auxss(t + ini, fin + 1 - ini,c);
     }
     return 1;
  }
  else {
     j = auxqs_particion(t,ini,fin,c);
     auxqs(t,ini,j-1,c);
     auxqs(t,j+1,fin,c);
     return 1;
  }
}

void strQuickSort(char **t,int size,cmp_string_ext c){
   if ( t && (size > 0) )
      auxqs(t,0,size-1,c);
}

void strShellSort(char **t,int size,cmp_string_ext c){
   if ( t && (size > 0) )
      auxss(t,size-1,c);
}

#endif
