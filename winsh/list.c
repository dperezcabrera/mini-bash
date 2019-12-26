/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/*@@@@@@@@@@@@   MODULO DE LISTA DE STRING   @@@@@@@@@@@@@@@*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

#ifndef LIST_C
#define LIST_C

#include "list.h"
#include "string_ext.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#ifndef TRUE
#define TRUE   1
#endif

#ifndef FALSE
#define FALSE  0
#endif

//   Funcion auxiliar para comparar dos elementos.
static int comparador_por_defecto(const char *a,const char *b) {

   // Compara dos elementos, devuelve '-1' si el
   // primero es el  menor, '0' si son iguales y
   // '1' si el primero es el menor.
   return (strcmp(a,b));
}

//   Funcion de nodo_s que crea nodo.
static nodo_s *nodo_s_new(element *s) {

   // Reserva el espacio necesario 
   // para almacenar un nodo.
   nodo_s *nod = (nodo_s *) malloc ( sizeof(nodo_s) );
   nod->sig = NULL ;
   nod->ant = NULL ;
   // Si 's' es  distinto de NULL, reserva  el espacio
   // necesario para poder copiar 's' y posteriormente
   // lo copia utilizando las funciones de la libreria
   // <string.h>.
   if ( s ) {
      nod->c = (char *) malloc (sizeof(char)*(strlen(s)+1)) ;
      strcpy(nod->c,s) ;
   }
   return nod ;
}

//   Funcion de list_it que crea un iterador.
static list_it* list_it_new(nodo_s *n) {
   
   // Se reserva espacio para almacenar un iterador.
   list_it *it = (list_it *) malloc (sizeof(list_it));
   it->elem = n ;
   return it ;
}


     ///////////////////////////////////////////////
     /////   Implementacion de las funciones   /////
     /////        publicas   de  list_t        /////
     ///////////////////////////////////////////////


//   Funcion de list_t que crea
// una lista  de string  vacia.
int list_new_f   (list_t *l,cmp comp) {

   // Comprueba que existe la lista.
   if ( l ) {;
      l->pri  =  NULL ;
      l->ult  =  NULL ;
      l->num_elem = 0 ;
      l->comparar = comp ;
      l->es_orden = TRUE ;
      return TRUE ;
   }
   else
     return FALSE ;
}

//   Funcion de list_t que crea
// una lista  de string  vacia.
int list_new   (list_t *l) {

   // Utiliza la funcion anterior.
   return list_new_f(l,comparador_por_defecto);
}

//   Funcion de list_t  destruye
// una lista de elementos vacia.
int list_delete(list_t *l) {
   
   nodo_s *it,*it2;
   it = l->pri ;
   // Comprueba que existe la lista.
   if ( l ) {
   // Bucle para borrar entera la lista de elementos.
      while ( it ) {
	 it2 = it->sig ;
	 free(it->c);
	 free(it) ;
	 it = it2 ;
      }
   // Inicializa  a los valores de los  campos
   // que tenian tras un list_new por defecto.
      l->pri  =  NULL ;
      l->ult  =  NULL ;
      l->es_orden = TRUE ;
      l->num_elem = 0 ;
      return TRUE ;
   }
   else
     return FALSE ;
}

//   Funcion  de list_t  que  borra un 
// elemento de la  lista de elementos.
int list_erase (list_t *l,element *s) {
   
   int salir = FALSE ;
   // Para optimizar si  la lista esta ordenada
   // utiliza  las variables  comp_s que guarda
   // la comparacion e is_sort para saber si la
   // lista 'l' esta ordenada.
   int comp_s = 2 ;
   int is_sort = l->es_orden ;
   nodo_s *it_ant,*it = l->pri ;
   // Primero comprueba que existe la lista y el elemento.
   if ( l && s ) {
   // Optimiza si la lista esta ordenada. 
      while( ( it != NULL ) && ( ! salir ) ) {
   // Trata de encontrar el elemento en 
   // tal caso, salir  valdra verdadero
   // y comprobara si lo  ha encontrado
   // o  no se encontraba en  la lista.
	 comp_s = l->comparar(s,it->c) ;
	 if (( comp_s == 0 ) || (( comp_s < 0 ) && is_sort ))
	   salir = TRUE ;
	 else {
	    it_ant = it ;   // it_ant: iterador anterior.
	    it  = it->sig ;
	 }
      }
      if ( it == NULL )
   // Caso en el que los elementos sean menores
   // que el buscado para borrarlo.
	return FALSE ;
      else {
   // Caso en el que se encuentra elemento a borrar.
	 if ( l->comparar(it->c,s) == 0 ) {
   // En caso en el cual sea el primer elemento.
	    if ( it == l->pri )
	      return list_pop_front(l) ;
	    else {
   // En caso en el cual sea el ultimo elemento.
	       if ( it == l->ult ) 
		 return list_pop_back(l) ;
   // En cualquier otro caso.
	       else {
		  it_ant->sig = it->sig ;
		  it->sig->ant = it_ant ;
		  free(it->c) ;
		  free(it) ;
		  l->num_elem-- ;
		  return TRUE ;
	       }
	    }
	 }
	 else
   // Caso en el cual encontramos un elemento mayor al 
   // buscado, entonces no  lo hemos encontrado puesto
   // que estan ordenados de mayor a menor.
	   return FALSE ;
      }
   }
   // Si o bien no existe la lista, o no existe el elemento.
   else
     return FALSE ;
}

//   Funcion de list_t que inserta un elemento
// de forma ordenada en la lista de elementos.
int list_insert(list_t *l,element *s) {
  
   nodo_s *n, *n_ant, *n_aux ;
   int salir = FALSE ;
   // Para comprobar que existe la lista y el elemento.
   if ( l && s ) {
   // Caso en el cual la lista este ordenada.
      if ( l->es_orden == TRUE ) {
   // Caso en el cual la lista no contenga ningun elemento.
	 if ( l->num_elem == 0 )
	   return list_push_front(l,s) ;
	 else {
   // Caso en el cual el elemento a insertar 
   // sea el menor de la lista.
	    if ( l->comparar(s,l->pri->c) <= 0 )
	      return list_push_front(l,s) ;
	    else {
   // Caso en el cual el elemento a insertar
   // sea el mayor de la  lista.
	       if ( l->comparar(l->ult->c,s) <= 0 )
		 return list_push_back(l,s) ;
   // En cualquier otro caso.
	       else {
		  n_aux = l->pri ;
		  while( ( n_aux ) && ( !salir ) ) {
		     if ( l->comparar(s,n_aux->c) <= 0 )
		       salir = TRUE ;
		     else {
			n_ant = n_aux ;  
			// n_ant: iterador anterior.
			n_aux  = n_aux->sig ;
		     }
		  }
   // Inserta el elemento en la lista.
		  n = nodo_s_new(s) ;
		  n_ant->sig = n ;
		  n->ant = n_ant ;
		  n->sig = n_aux ;
		  n_aux->ant = n ;
		  l->num_elem++  ;
		  return TRUE ;
	       } 
	    }
	 }
      }
      else {
   // Inserta al principio  y ordena la lista de 
   // elementos de modo que esta queda ordenada.
	 list_push_front(l,s) ;
	 return list_sort (l) ;
      }
   // Si o bien no existe la lista, o no existe el elemento.
   }
   else
     return FALSE ;
}

//   Funcion de list_t para devolver el numero
// de elementos de la lista.
int list_size(list_t *l) {

   // Simplemente devuelve el campo de 
   // numero de elementos.
   if ( l )
      return l->num_elem;
   else
      return -1;
}

//   Funcion de list_t para insertar al 
// principio de  la lista de elementos.
int  list_push_front(list_t *l,element *s) {

   nodo_s *n ;
   // Comprobamos que existe la lista y el elemento.
   if ( l && s ) {
   // Si almenos hay un elemento en la lista.
      if ( l->num_elem > 0 ) {
   // Para modificar el campo  es_orden en el caso
   // en el cual deje de verificarse el invariante
   // de estar ordenado.
	 if ( l->comparar(l->pri->c,s) < 0 )
	   l->es_orden = FALSE ;
	 n = nodo_s_new(s) ;
	 l->num_elem++ ;
	 n->sig = l->pri ;
	 l->pri->ant = n ;
	 l->pri = n ;
	 return TRUE ;
      }
   // En el caso en el cual sea 's' el primer
   // elemento a insertar.
      else {
	 n = nodo_s_new(s) ;
	 l->num_elem = 1 ;
	 l->pri = n ;
	 l->ult = n ;
	 return TRUE ;
      }
   }
   // En caso en el cual no exista la 
   // lista o no  exista el elemento.
   else
     return FALSE ;
}

//   Funcion de  list_t para insertar 
// al final de la lista de elementos.
int  list_push_back (list_t *l,element *s) {
   
   nodo_s *n ;
   // Comprobamos que existe la lista y el elemento.
   if ( l && s ) {
   // Si almenos hay un elemento en la lista.
      if ( l->num_elem > 0 ) {
   // Para modificar  el campo es_orden en el caso
   // en el cual deje de verificarse el invariante
   // de estar ordenado.
	 if ( l->comparar(l->ult->c,s) > 0 )
	   l->es_orden = FALSE ;
	 n = nodo_s_new (s) ;
	 l->num_elem++ ;
	 l->ult->sig = n ; 
	 n->ant = l->ult ;
	 l->ult = n  ;
	 return TRUE ;
      }
   // En el caso en el  cual sea 's' 
   // el primer elemento a insertar.
      else {
	 n = nodo_s_new(s) ;
	 l->num_elem = 1 ;
	 l->pri = n  ;
	 l->ult = n  ;
	 return TRUE ;
      }
   }
   else
     return FALSE ;
}

//   Funcion de list_t para borrar
// el primer elemento de la lista.
int  list_pop_front(list_t *l) {
   
   // Comprobamos que existe la lista.
   if ( l ) {
   // Almenos  debe haber  un elemento 
   // en la lista para poder borrarlo.
      if ( l->num_elem > 0 ) {
    // Si solo hay un elemento destruye
    // la lista al completo.
	 if ( l->num_elem == 1 )
	   return list_delete(l) ;
   // Caso en el cual hay varios elementos en la
   // lista ahora solo debe  eliminar el ultimo.
	 else {
	    nodo_s  *it = l->pri ;
	    l->pri = l->pri->sig ;
	    l->pri->ant = NULL ;
	    free(it->c) ;
	    free(it) ;
	    l->num_elem-- ;
	    return TRUE ;
	 }
      }
   // En el caso en el cual no hay ningun elemento 
   // la funcion se limita a devolver FALSE.
      else
	return FALSE ;
   }
   else
     return FALSE ;
}

//   Funcion de list_t para borrar el ultimo 
// elemento de la lista.
int  list_pop_back (list_t *l) {
   
   // Comprobamos que existe la lista.
   if ( l ) {
   // Almenos debe haber un elemento en la lista
   // para poder borrarlo.
      if ( l->num_elem > 0 ) {
   // Si solo hay un elemento destruye 
   // la lista al completo.
	 if ( l->num_elem == 1 )	
	   return list_delete(l) ;
   // Caso en el cual hay varios elementos en la
   // lista, ahora solo debe eliminar el ultimo.
	 else {
	    nodo_s  *it = l->ult ;
	    l->ult = l->ult->ant ;
	    l->ult->sig = NULL ;
	    free(it->c) ;
	    free(it) ;
	    l->num_elem-- ;
	    return TRUE ;
	 }
      }
   // En el caso en el que no hay ningun elemento 
   // la funcion se limita a devolver FALSE.
      else
	return FALSE ;
   }
   else
     return FALSE ;
}

//   Funcion de list_t que devuelve el 
// primer elemento de la lista.
element*list_front(list_t *l) {
  
   // Comprobamos que existe la lista.
   if ( l ) {
   // Almenos debe tener un elemento la lista
   // para poder devolver el primero.
      if ( l->num_elem > 0 )
	return l->pri->c ;
   // Devuelve NULL si no hay ningun elemento.
      else
	return NULL ;
   }
   else
     return NULL ;
}

//   Funcion de list_t que devuelve 
// el ultimo elemento  de la lista.
element*list_back (list_t *l) {
   
   // Comprobamos que existe la lista.
   if ( l ) {
   // Almenos debe tener un elemento la lista
   // para poder devolver el ultimo.
      if ( l->num_elem > 0 )
	return l->ult->c ;
   // Devuelve NULL si no hay ningun elemento.
      else
	return NULL ;
   }
   else
     return NULL ;
}

//   Funcion de list_t para saber 
// si la lista esta ordenada.
int  list_are_sort (list_t *l) {
   
   if ( l )
     return l->es_orden ;
   else
     return FALSE ;
}

//   Funcion de list_t para ordenar una lista.
int  list_sort (list_t *l) {
   
   element **t ;
   int num_e,i ;
   // Comprueba que existe la lista.
   if ( l ) {
   // Comprueba que no esta ordenada.
      if ( !l->es_orden ) {
   // Crea una tabla de elementos para ordenarlos.
         t = list_to_array( l );
	 num_e = l->num_elem;
   // Ordena la tabla de elemenetos.
	 strQuickSort(t,num_e,l->comparar);
   // Devuelve los elementos.
         list_from_array(l, t, num_e);
	 for ( i = 0 ; i < num_e ; i++){
	    free(t[i]);
	 }
	 free(t);
   // Como ha ordenado la lista, devuelve TRUE.
	 return TRUE ;
      }
   // Como la lista estaba ordenada, devuelve FALSE.
      else
        return FALSE ;
   }
   // Como no existe lista no la puede ordenar y devuelve FALSE.
   else
     return FALSE ;
}

//   Funcion de list_t que devuelve una tabla con sus elementos.
element** list_to_array(list_t *l){
   element **t ;
   nodo_s *n;
   int i, num_e ;
   if ( l && (l->num_elem > 0) ){
      n  = l->pri;
      num_e = l->num_elem;
      t = (element **)malloc(sizeof(element *)*num_e);
      i = 0;
      while ( n ) {
	 if (n->c)
	   t[i] = strdup(n->c);
	 n = n->sig ;
	 i++;
      }
      return t;
   }
   else
     return NULL;
}

//   Funcion de list_t para insertar los elementos
// desde un array.
int list_from_array(list_t *l, element **t,int num_e){
   int i = 0 ;
   if ( l && t && ( num_e > 0) ){
      list_delete( l );
   // Introduce los elementos ordenados en la lista.
      while( i < num_e ) {
	list_push_back(l,t[i]) ;
	i++;
      }
      return TRUE;
   }
   else
      return FALSE;
}

//   Funcion de list_t que copia una lista en otra.
int  list_copy(list_t *l,list_t *p) {
   
   list_it it;
   if ( l && p ) {
      list_delete(l);
      if ( list_size(p) > 0 ) {
	 list_it_copy(&it,list_first(p));
	 while ( !list_it_is_last(&it) ) {
	    list_push_back(l,list_it_element(&it)) ;
	    list_it_next(&it);
	 }
	 list_push_back(l,list_it_element(&it)) ;
	 return TRUE;
      }
      else
	return FALSE;
   }
   else
     return FALSE;
}


//   Funcion de list_t que  devuelve  un iterador 
// el cual apunta al primer elemento de la lista.
list_it* list_first (list_t *l) {
   
   if ( l ) 
     return list_it_new(l->pri) ;
   else
     return NULL ;
}

//   Funcion de list_t que  devuelve  un iterador
// el cual apunta al primer elemento de la lista.
list_it* list_last  (list_t *l) {

   if ( l )
     return list_it_new(l->ult);
   else
     return NULL ;
}

//   Funcion de list_t para depurar la estructura list_t.
void list_show(list_t *l) {
   
   nodo_s *n = l->pri;
   int i = 0 ;
   if ( l ) {
   // Recorre la lista hasta el final y muestra los elementos.
      while ( n ) {
	 i++;
	 if (n->c)
	   printf("<%s>",n->c);
	 n = n->sig ;
      }
      if ( i != l->num_elem ) {
	 printf("ERROR: numero de elementos erroneo\n") ;
      }
   }
}


     ///////////////////////////////////////////////
     /////   Implementacion de las funciones   /////
     /////        publicas  de  list_it        /////
     ///////////////////////////////////////////////

//   Funcion de list_it que devuelve el 
// elemento  al que  apunta el  iterador.
element*list_it_element (list_it *it) {

   // Comprueba si existe el iterador.
   if ( it ) {
   // Comprueba si el iterador apunta a algo.
      if ( it->elem )
	return it->elem->c ;
      else
	return NULL;
   }
   // Si no existe el iterador.
   else
     return NULL;
}

//   Funcion  de list_it  que  devuelve TRUE
// si apunta al ultimo elemento de la lista,
// y devuelve FALSE en caso contrario.
int  list_it_is_last (list_it *it) {
  
   // Comprueba si existe el iterador.
   if ( it ) {
   // Comprueba si el iterador apunta a algo.
      if ( it->elem )
	return ( it->elem->sig == NULL ) ;
      else
   // En el caso en el cual  no apunte a nada
   // devolvera TRUE, para evitar que avance.
	return TRUE ;
   }
   // Si no existe el iterador devuelve NULL.
   else
     return TRUE ;
}


//   Funcion de list_s_it que  devuelve TRUE
// si apunta al primer elemento de la lista,
// y devuelve FALSE en caso contrario.
int  list_it_is_first(list_it *it) {
   
   // Comprueba si existe el iterador.             
   if ( it ) {
   // Comprueba si el iterador apunta a algo.
      if ( it->elem )
	return ( it->elem->ant == NULL ) ;
      else
   // En el caso en el cual  no apunte a nada
   // devolvera TRUE, para evitar que avance.
	return TRUE ;
   }
   // Si no existe el iterador devuelve TRUE.
   else
     return TRUE ;
}

//   Funcion  de list_s_it  que copia 
// el segundo iterador en el primero.
int  list_it_copy (list_it *it,list_it *jt) {
   
   // Comprueba si existen los dos iteradores.
   if ( it && jt ) {
      it->elem = jt->elem ;
   // Copia el apuntador, pero si
   // este no apunta a nada, esta
   // funcion devuelve FALSE.
      return  ( jt->elem != NULL ) ;
   }
   // Si no existe alguno de los iteradores devuelve FALSE.
   else
     return FALSE ;
}

//   Funcion de list_it que devuelve TRUE si
// los iteradores apuntan  al mismo elemento
// de una lista.
int  list_it_equal(list_it *it,list_it *jt) {
   
   // Comprueva que existen ambos iteradores.
   if ( it && jt ) 
   // Compara los apuntadores de cada iterador.
     return ( it->elem == jt->elem ) ;
   // Si no existe alguno de los iteradores devuelve FALSE.
   else
     return FALSE ;
}

//   Funcion de  list_it que  modifica  el  puntero  para que 
// apunte al siguiente elemento de la lista ( si es posible )
// si es posible devolvera TRUE  en caso contrario  devolvera
// FALSE.
int  list_it_next    (list_it *it) {
   
   // Comprueba que
   if ( it ) {
   // Comprueba que apunta a algo.
      if ( it->elem ) {
   // Comprueba que existe el siguiente.
	 if ( it->elem->sig ) {
   // Avanza y devuelve TRUE porque ha podido avanzar.
	    it->elem = it->elem->sig ;
	    return TRUE ;
	 }
   // En caso contrario  devuelve FALSE por
   // que no ha podido avanzar el iterador.
	 else
	   return FALSE ;
      }
      else
   // En caso contrario  devuelve FALSE por
   // que no ha podido avanzar el iterador.
	return FALSE ;
   }
   else
     return FALSE ;
}

//   Funcion de  list_it que  modifica el puntero  para  que
// apunte al anterior elemento de la lista ( si es posible )
// si es posible devolvera TRUE  en caso contrario devolvera
// FALSE. 
int  list_it_previous(list_it *it) {
   
   // Comprueba si existe el iterador.
   if ( it ) {
   // Comprueba si apunta a algo.
      if ( it->elem ) {
   // Comprueba que existe el anterior.
	 if ( it->elem->ant ) {
   // Avanza y devuelve TRUE porque ha podido avanzar. Ã
	    it->elem = it->elem->ant ;
	    return TRUE ;
	 }
   // En caso contrario  devuelve FALSE por
   //  que no ha podido avanzar el iterador.
	 else
	   return FALSE ;
      }
   // En caso contrario  devuelve FALSE por
   // que no ha podido avanzar el iterador.
      else
	return FALSE ;
   }
   // En caso en el cual no exista el iterador devuelve FALSE.
   else
     return FALSE ;
}

#endif
/* 
  $Id: list.c
  725 lineas
  v 2.0 24/3/2005 12:30:40 
  David Perez Cabrera
  Exp $ 
*/
