# -*- Makefile -*-
#
# Reglas comunes para Borland C++ Compiler
#
# $Id: makeincl.inc 848 2004-10-19 16:54:59Z dsevilla $
#

.AUTODEPEND

BCCROOT=c:\Borland\bcc55

CPP        = bcc32

CCLINKOPT= -lGn -L$(BCCROOT)\Lib -tWM
CCOPTS=-I$(BCCROOT)\Include

#Compile flags:
CPPFLAGS= $(CCOPTS) $(CCLINKOPT)
