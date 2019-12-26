#ifndef __SMALLSH_H
#define __SMALLSH_H

#include      "list.h"

#include     <ctype.h>
#include    <dirent.h>
#include     <errno.h>
#include     <fcntl.h>
#include   <fnmatch.h>
#include      <glob.h>
#include       <grp.h>
#include   <ncurses.h>
#include       <pwd.h>
#include    <setjmp.h>
#include    <signal.h>
#include     <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include  <sys/mman.h>
#include  <sys/stat.h>
#include <sys/types.h>
#include  <sys/wait.h>
#include      <time.h>
#include    <unistd.h>
#include      <wait.h>

/* Definición del buffer de tokens */
#include "gettok.h"

/* Funciones del usuario */
#include "userfn.h"

/* Funciones del buffer */
#include "buffer.h"

#define MAXARG  1024

#define OR_S   "$||"
#define AND_S  "$&&"
#define PIPE_S  "$|"

#define OP_INIT   0
#define OP_PIPE   1
#define OP_ARROBA 2
#define OP_LOGICO 4
#define OP_MASK  63

#define TERMINA_BIEN 0
#define TERMINA_MAL  1
#define NOT_FOUND   -1

#define FOREGROUND 0
#define BACKGROUND 1

enum PALABRAS_RESERVADAS_SMALLSH { 
     FALSO,VERDADERO,BG,CD,DEFINICION,EXIT,EXPORT,
     FG,HEAD, JOBS, LS, SET, TAIL, UNSET,NINGUNA};

void procline(struct TokBuf*);
int runcommand(char **,int,int,unsigned int);

#endif

/* $Id: smallsh.h 845 2004-10-19 16:08:22Z dsevilla $ */
