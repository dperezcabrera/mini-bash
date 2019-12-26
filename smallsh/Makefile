#! /usr/bin/make -f
# Fichero Makefile para construir el ejecutable smallsh

CFLAGS=-g -Wall
LDLIBS=-lfl -lncurses# Para flex

OBJECTS=smallsh.o gettok.o procline.o runcom.o buffer.o userfn.o list.o string_ext.o shell.o

smallsh: $(OBJECTS)

gettok.o: gettok.c

procline.o: procline.c

runcom.o: runcom.c

smallsh.o: smallsh.c

buffer.o: buffer.c

userfn.o: userfn.c

list.o: list.c

string_ext.o: string_ext.c

shell.c: shell.l

clean:
	rm -rf *~ $(OBJECTS) shell.c smallsh core

# $Id: Makefile 845 2004-10-19 16:08:22Z dsevilla $
