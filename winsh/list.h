/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/*@@@@@@@@@@@@   MODULO DE LISTA DE STRING   @@@@@@@@@@@@@@@*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
#ifndef LIST_H
#define LIST_H

typedef char element;

struct nodo_list {
  element *c;
  struct nodo_list *sig;
  struct nodo_list *ant;
};

typedef struct nodo_list nodo_s;

struct iterator_s {
  nodo_s *elem ;
};

typedef int(*cmp)(const char*,const char*);

struct listas  {
  cmp comparar ;
  nodo_s  *pri ;
  nodo_s  *ult ;
  int num_elem ;
  int es_orden ;
};

typedef struct listas list_t;
typedef struct iterator_s list_it;

   ////////////////////////////////////////////
   // funciones publicas de listas de string //
   ////////////////////////////////////////////

int  list_new   (list_t *l);// ok
int  list_new_f (list_t *l,cmp c);// ok
int  list_delete(list_t *l);// ok
int  list_size  (list_t *l);// ok
int  list_erase (list_t *l,element *s);// ok
int  list_insert(list_t *l,element *s);// ok
int  list_push_front(list_t *l,element *s);// ok
int  list_push_back (list_t *l,element *s);// ok
int  list_pop_front(list_t *l);// ok
int  list_pop_back (list_t *l);// ok
int  list_are_sort (list_t *l);// ok
int  list_sort (list_t *l);// ok
int  list_copy (list_t *l,list_t *p);
int  list_from_array(list_t*,element**,int);// ok
element* list_front(list_t *l);// ok
element* list_back (list_t *l);// ok
element**list_to_array(list_t *l);// ok
void     list_show (list_t *l);// ok
list_it* list_first (list_t *l);// ok
list_it* list_last  (list_t *l);// ok

////////////////////////////////////////////////////////
// funciones publicas de iterador de listas de string //
////////////////////////////////////////////////////////

element*list_it_element (list_it *it);// ok
int  list_it_is_last (list_it *it);// ok
int  list_it_is_first(list_it *it);// ok
int  list_it_copy (list_it *it,list_it *jt);// ok
int  list_it_equal(list_it *it,list_it *jt);// ok
int  list_it_next    (list_it *it);// ok
int  list_it_previous(list_it *it);// ok

#endif
/* 
  $Id: list.h
  77 lineas 
  800 lineas en total list.h + list.c
  v 1.2 14/10/2004 19:30:40 
  David Perez Cabrera
  Exp $ 
*/
