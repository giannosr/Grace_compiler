%{
#include "ast.hpp"
#include "lexer.hpp"
#include "parser.hpp"

#define T_eof 0

int lineno = 1;
%}

L [A-Za-z]
D [0-9]
W [ \t\n\r]
X [0-9A-Fa-f]
P [^\x00-\x1F\x7F\'\"\\]
C {P}|"\\"([ntr0\\\'\"]|x{X}{X})

%option noyywrap

%%

"and"     { return T_and; }
"char"    { return T_char; }
"div"     { return T_div; }
"do"      { return T_do; }
"else"    { return T_else; }
"fun"     { return T_fun; }
"if"      { return T_if; }
"int"     { return T_int; }
"mod"     { return T_mod; }
"not"     { return T_not; }
"nothing" { return T_nothing; }
"or"      { return T_or; }
"ref"     { return T_ref; }
"return"  { return T_return; }
"then"    { return T_then; }
"var"     { return T_var; }
"while"   { return T_while; }

"<-"                 { return T_assign; }
[\(\)\[\]\{\}\,\;\:] { return yytext[0]; }

"<="             { return T_leq; }
">="             { return T_geq; }
[\+\-\*\=\#\<\>] { return yytext[0]; }

{L}({L}|{D}|_)*  { yylval.id_name = strdup(yytext); return T_id; }
{D}+             { yylval.num = atoi(yytext); return T_uint_const; }
"\'"{C}"\'"      { return T_char_const; }
"\""{C}+"\""     { return T_str_const; }

[ \t\r]+         { /* do nothing (whitespace except new line) */ }

(\$([^\$\n].*)?)?\n      { ++lineno; /* single line comment (not begining with $$) or new line, increase line count */ }
"$$"([^\$]|\$[^\$])*"$$" { /* do nothing (multiline comment) */ }

. { fprintf(stderr, "Lexer Error: character %c is considered incorrect. In line %d (remove your multiline comments to find that)\n", yytext[0], lineno); exit(1); }

%%
