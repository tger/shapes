/* This file is part of Shapes.
 *
 * Shapes is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * Shapes is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Shapes.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright 2008 Henrik Tidefelt
 */

%{

#include "pagescanner.h"
#include <string.h>
#include <iostream>
#include <iomanip>

%}

WhiteSpace [ \t\n\r]

Float [+-]?[0-9]*[.]?[0-9]*

Name [/]{Regular}*


IndirectRef {PlainInteger}{WhiteSpace}+{PlainInteger}{WhiteSpace}*"R"
IndirectDef {PlainInteger}{WhiteSpace}+{PlainInteger}{WhiteSpace}*"obj"

%option c++
%option noyywrap

%option yyclass="PageScanner"

%x Copy

%%

<Copy>(.|[\n]) { (*yyout) << *yytext ; }

({Float}{WhiteSpace}+){5}{Float}{WhiteSpace}*"cm" {
	char * end;
	double dummy;
	dummy = strtod( yytext, & end );
	if( dummy != 1 )
		{
			throw( "Expected pure translation in the initial coordinate transform matrix" );
		}
	dummy = strtod( end, & end );
	if( dummy != 0 )
		{
			throw( "Expected pure translation in the initial coordinate transform matrix" );
		}
	dummy = strtod( end, & end );
	if( dummy != 0 )
		{
			throw( "Expected pure translation in the initial coordinate transform matrix" );
		}
	dummy = strtod( end, & end );
	if( dummy != 1 )
		{
			throw( "Expected pure translation in the initial coordinate transform matrix" );
		}
	x0 = strtod( end, & end );
	y0 = strtod( end, & end );
	(*yyout) << yytext << endl ;
}

"q" ;
"[]0 d" ;
"0 J" ;
{Float}{WhiteSpace}*"w" ;
"S" ;

"Q" {
	if( state == 3 )
		{
			BEGIN( Copy );
		}
}

{Float}{WhiteSpace}*{Float}{WhiteSpace}*"m" {
	char * end;
	double dummy;
	dummy = strtod( yytext, & end );
	ytmp = strtod( end, & end );
}

{Float}{WhiteSpace}*{Float}{WhiteSpace}*"l" {
	char * end;
	strtod( yytext, & end );
	double tmp = strtod( end, & end ) - 3;
	switch( state )
		{
		case 0:
			height = tmp;
			break;
		case 1:
			depth = tmp;
			break;
		case 2:
			width = tmp;
			break;
		default:
			throw( "Surprising state in pagescanner" );
		}
	++state;
}

{WhiteSpace}+ ;

. { throw( string( "Page scanner found unrecognized token: " ) + yytext ); }

%%
/* The closing %% above marks the end of the Rules section and the beginning
 * of the User Subroutines section. All text from here to the end of the
 * file is copied verbatim to the end of the generated lex.pdf.c file.
 * This section is where you put definitions of helper functions.
 */


