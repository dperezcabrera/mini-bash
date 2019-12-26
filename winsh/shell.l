%{
#include "gettok.h"
%}

TODOS_DOBLE	[^\"]*
TODOS_SIMPLE	[^']*

%%

\\[ \t]+			trataCadena( yytext +1 );
\"{TODOS_DOBLE}\"		trataCadCom( yytext );
'{TODOS_SIMPLE}'		trataCadCom( yytext );
'				trataToken( QUOTE, yytext );
&				trataToken( AMPERSAND, yytext );
;				trataToken( SEMICOLON, yytext );
\|\|                            trataToken( OR, yytext );
&&                              trataToken( AND, yytext );
\|				trataToken( PIPE, yytext );
`				trataToken( BACKQUOTE, yytext );
\>				trataToken( MAYOR, yytext );
\<				trataToken( MENOR, yytext );
\>\>				trataToken( MAYORMAYOR, yytext );
2\>				trataToken( DOSMAYOR, yytext );
2\>\>				trataToken( DOSMAYORMAYOR, yytext );

@[0-9]+                         trataToken( ARROBA, yytext );

[ \t]+				trataToken( SPACE, yytext );

.				trataChar( yytext[0] );

%%

/* $Id: shell.l 910 2005-01-11 13:28:59Z dsevilla $ */
