%{
#include <cstdio>
#include <cstdlib>
#include "ast.hpp"
#include "lexer.hpp"

print_align align;
%}

%token T_and     "and"
%token T_char    "char"
%token T_div     "div"
%token T_do      "do"
%token T_else    "else"
%token T_fun     "fun"
%token T_if      "if"
%token T_int     "int"
%token T_mod     "mod"
%token T_not     "not"
%token T_nothing "nothing"
%token T_or      "or"
%token T_ref     "ref"
%token T_return  "return"
%token T_then    "then"
%token T_var     "var"
%token T_while   "while"

%token T_leq     "<="
%token T_geq     ">="
%token T_assign  "<-"

%token<id_name> T_id
%token<num> T_uint_const "int_const"
%token T_char_const "char_const"
%token T_str_const  "string_literal"

%left "or"
%left "and"
%left "not"
%nonassoc '=' '#' '>' '<' "<=" ">="
%left '+' '-'
%left '*' "div" "mod"
%left UPLUS UMINUS

%union {
	const char      *id_name;
	int             num;
	Local_def       *ldef;
	Header          *hdr;
	Local_def_list  *ldlist;
	Block           *blk;
	Id_list         *idlist;
	Data_type       *dtype;
	Type            *type;
	Array_tail_decl *tail;
	Fpar_type       *fpt;
	Ret_type        *rt;
	Fpar_def_list   *fpdl;
	Fpar_def        *fpd;
}

%type<ldef>   local_def
%type<ldef>   func_def
%type<ldef>   func_decl
%type<ldef>   var_def
%type<hdr>    header
%type<ldlist> local_def_list
%type<blk>    block
%type<idlist> identifier_list
%type<dtype>  data_type
%type<type>   type
%type<tail>   array_tail_decl
%type<fpt>    fpar_type
%type<rt>     ret_type
%type<fpdl>   fpar_def_list
%type<fpd>    fpar_def


%expect 1

%define parse.error detailed

%%

program:
  func_def { std::cout << "AST:\n" << *$1 << std::endl; }
;

func_def:
  header local_def_list block { $$ = new Func_def($1, $2, $3); }
;

local_def_list: /* nothing */ { $$ = new Local_def_list(); }
| local_def_list local_def    { $1->append($2); $$ = $1; }
;

header: 
  "fun" T_id '(' ')' ':' ret_type               { $$ = new Header(new Id($2), nullptr, $6); }
| "fun" T_id '(' fpar_def_list ')' ':' ret_type { $$ = new Header(new Id($2), $4, $7); }
;

fpar_def_list:
  fpar_def                   { $$ = new Fpar_def_list($1); }
| fpar_def_list ';' fpar_def { $1->append($3); $$ = $1; }
;

fpar_def:
  identifier_list ':' fpar_type       { $$ = new Fpar_def(false, $1, $3); }
| "ref" identifier_list ':' fpar_type { $$ = new Fpar_def(true, $2, $4); }
;

identifier_list:
  T_id                     { $$ = new Id_list(new Id($1)); }
| identifier_list ',' T_id { $1->append(new Id($3)); $$ = $1; }
;

data_type:
  "int"  { $$ = new Data_type("int"); }
| "char" { $$ = new Data_type("char"); }
;

type:
  data_type                 { $$ = new Type($1); }
| data_type array_tail_decl { $$ = new Type($1, $2); }
;

array_tail_decl:
  '[' "int_const" ']'                 { $$ = new Array_tail_decl($2); }
| array_tail_decl '[' "int_const" ']' { $1->append($3); $$ = $1; }
;

ret_type:
  data_type { $$ = new Ret_type($1); }
| "nothing" { $$ = new Ret_type(nullptr); }
;

fpar_type:
  data_type                         { $$ = new Fpar_type($1, false, nullptr); }
| data_type '[' ']'                 { $$ = new Fpar_type($1, true, nullptr); }
| data_type array_tail_decl         { $$ = new Fpar_type($1, false, $2); }
| data_type '[' ']' array_tail_decl { $$ = new Fpar_type($1, true, $4); }
;

local_def:
  func_def  { $$ = $1; }
| func_decl	{ $$ = $1; }
| var_def   { $$ = $1; }
;

func_decl:
  header ';' { $$ = $1; }
;

var_def: 
  "var" identifier_list ':' type ';' { $$ = new Var_def($2, $4); }
;

stmt:
  ';'
| l_value "<-" expr ';'
| block
| func_call ';'
| "if" cond "then" stmt
| "if" cond "then" stmt "else" stmt
| "while" cond "do" stmt
| "return" ';'
| "return" expr ';'
;

block:
  '{' stmt_list '}' { $$ = new Block(); }
;

stmt_list: /* nothing */
| stmt_list stmt
;

func_call:
  T_id '(' ')'
| T_id '(' expr_list ')'
;

expr_list:
  expr
| expr_list ',' expr
;

l_value:
  T_id
| "string_literal"
| l_value '[' expr ']'
;

expr:
  "int_const"
| "char_const"
| l_value
| '(' expr ')'
| func_call
| '+' expr %prec UPLUS
| '-' expr %prec UMINUS
| expr '+' expr
| expr '-' expr
| expr '*' expr
| expr "div" expr
| expr "mod" expr
;

cond:
  '(' cond ')'
| "not" cond
| cond "and" cond
| cond "or" cond
| expr '=' expr
| expr '#' expr
| expr '<' expr
| expr '>' expr
| expr "<=" expr
| expr ">=" expr
;

%%

void yyerror(const char *msg) {
    fprintf(stderr, "Line %d: %s\n", lineno, msg);
    exit(2);
}

int main() {
    int res = yyparse();
    if(res == 0) printf("Parsing Success!\n");
    else         printf("Something went wrong...\n");
    return res;
}
