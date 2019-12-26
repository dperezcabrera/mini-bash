#ifndef __WINSH_H
#define __WINSH_H

#include <ctype.h>
#include <direct.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <windows.h>

/* Funciones de esta version del shell */
#include "wsh.h"

#include "list.h"

/* Definición del buffer de tokens */
#include "gettok.h"

/* Funciones del usuario */
#include "userfn.h"

/* Funciones del buffer */
#include "buffer.h"

/* Funciones utiles para el shell */
#include "shlib.h"

/* Funciones para llamar al sistema */
#include "shell_system.h"
#define MAXCMD 512
#define MAXARG 512

#define FOREGROUND 0
#define BACKGROUND 1

// En procline.c
void procline(struct TokBuf*);

// En runcom.c
int runcommand(char **,int,int,int);
int System_Exec(char **,int,int);
int Run_Only_A_Comand( int, char **);

#endif

/* $Id: winsh.h 848 2004-10-19 16:54:59Z dsevilla $ */
