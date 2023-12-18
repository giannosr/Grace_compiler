%{
#include <cstdio>
#include <cstdlib>

#include "ast.hpp"
#include "runtime_syms.cpp"
#include "lexer.hpp"

print_align align;

symbol_table st;
ll_symbol_table ll_st;

const char *INT  = "int";
const char *CHAR = "char";

/* Basic Types (usefull in some AST operations) */
Type Int_t(new Data_type(INT));
Type Char_t(new Data_type(CHAR));
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
%token<chr> T_char_const "char_const"
%token<str> T_str_const  "string_literal"

%left<op> "or"
%left<op> "and"
%left<op> "not"
%nonassoc<op> '=' '#' '>' '<' "<=" ">="
%left<op> '+' '-'
%left<op> '*' "div" "mod"
%left UPLUS UMINUS

%union {
	const char         *id_name;
	unsigned long long num;
	const char         *str;
	const char         *chr;
	Local_def          *ldef;
	Func_def           *fdef;
	Header             *hdr;
	Local_def_list     *ldlist;
	Block              *blk;
	Id_list            *idlist;
	Data_type          *dtype;
	Type               *type;
	Array_tail_decl    *tail;
	Fpar_type          *fpt;
	Ret_type           *rt;
	Fpar_def_list      *fpdl;
	Fpar_def           *fpd;
	Stmt               *stm;
	Func_call          *fc;
	L_value            *lv;
	Expr               *exp;
	Expr_list          *elist;
	Cond               *cnd;
	char               op;
}

%type<ldef>   local_def func_decl var_def
%type<fdef>   func_def 
%type<hdr>    header
%type<ldlist> local_def_list
%type<blk>    block stmt_list
%type<idlist> identifier_list
%type<dtype>  data_type
%type<type>   type
%type<tail>   array_tail_decl
%type<fpt>    fpar_type
%type<rt>     ret_type
%type<fpdl>   fpar_def_list
%type<fpd>    fpar_def
%type<stm>    stmt
%type<fc>     func_call
%type<elist>  expr_list
%type<lv>     l_value
%type<exp>    expr
%type<cnd>    cond


%expect 1

// %define parse.error detailed

%%

program:
  func_def {
    // std::cout << "AST:\n" << *$1 << std::endl;
    $1->sem();
    $1->set_main();
    $1->llvm_compile_and_dump();
  }
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
| "ref" identifier_list ':' fpar_type { $$ = new Fpar_def(true,  $2, $4); }
;

identifier_list:
  T_id                     { $$ = new Id_list(new Id($1)); }
| identifier_list ',' T_id { $1->append(new Id($3)); $$ = $1; }
;

data_type:
  "int"  { $$ = new Data_type(INT); }
| "char" { $$ = new Data_type(CHAR); }
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
| data_type '[' ']'                 { $$ = new Fpar_type($1, true,  nullptr); }
| data_type array_tail_decl         { $$ = new Fpar_type($1, false, $2); }
| data_type '[' ']' array_tail_decl { $$ = new Fpar_type($1, true,  $4); }
;

local_def:
  func_def  { $$ = $1; }
| func_decl { $$ = $1; }
| var_def   { $$ = $1; }
;

func_decl:
  header ';' { $$ = $1; }
;

var_def: 
  "var" identifier_list ':' type ';' { $$ = new Var_def($2, $4); }
;

stmt:
  ';'                               { $$ = new Empty_stmt(); }
| l_value "<-" expr ';'             { $$ = new Assign($1, $3); }
| block                             { $$ = $1; }
| func_call ';'                     { $$ = $1; }
| "if" cond "then" stmt             { $$ = new If($2, $4, nullptr); }
| "if" cond "then" stmt "else" stmt { $$ = new If($2, $4, $6); }
| "while" cond "do" stmt            { $$ = new While($2, $4); }
| "return" ';'                      { $$ = new Return(nullptr); }
| "return" expr ';'                 { $$ = new Return($2); }
;

block:
  '{' stmt_list '}' { $$ = $2; }
;

stmt_list: /* nothing */ { $$ = new Stmt_list(); }
| stmt_list stmt         { $1->append($2); $$ = $1; }
;

func_call:
  T_id '(' ')'           { $$ = new Func_call(new Id($1), nullptr); }
| T_id '(' expr_list ')' { $$ = new Func_call(new Id($1), $3); }
;

expr_list:
  expr               { $$ = new Expr_list($1); }
| expr_list ',' expr { $1->append($3); $$ = $1; }
;

l_value:
  T_id                 { $$ = new L_value(new Id($1), nullptr, nullptr, nullptr); }
| "string_literal"     { $$ = new L_value(nullptr, $1, nullptr, nullptr); }
| l_value '[' expr ']' { $$ = new L_value(nullptr, nullptr, $1, $3); }
;

expr:
  "int_const"           { $$ = new Int_const($1); }
| "char_const"          { $$ = new Char_const($1); }
| l_value               { $$ = $1; }
| '(' expr ')'          { $$ = $2; }
| func_call             { $$ = $1; }
| '+' expr %prec UPLUS  { $$ = new UnOp($1, $2); }
| '-' expr %prec UMINUS { $$ = new UnOp($1, $2); }
| expr '+' expr         { $$ = new BinOp($1, $2, $3); }
| expr '-' expr         { $$ = new BinOp($1, $2, $3); }
| expr '*' expr         { $$ = new BinOp($1, $2, $3); }
| expr "div" expr       { $$ = new BinOp($1, $2, $3); }
| expr "mod" expr       { $$ = new BinOp($1, $2, $3); }
;

cond:
  '(' cond ')'    { $$ = $2; }
| "not" cond      { $$ = new NotCond($2); }
| cond "and" cond { $$ = new BinCond($1, $2, $3); }
| cond "or" cond  { $$ = new BinCond($1, $2, $3); }
| expr '=' expr   { $$ = new BinOpCond($1, $2, $3); }
| expr '#' expr   { $$ = new BinOpCond($1, $2, $3); }
| expr '<' expr   { $$ = new BinOpCond($1, $2, $3); }
| expr '>' expr   { $$ = new BinOpCond($1, $2, $3); }
| expr "<=" expr  { $$ = new BinOpCond($1, $2, $3); }
| expr ">=" expr  { $$ = new BinOpCond($1, $2, $3); }
;

%%

/* necessary because C++ is dumb */
bool check_fpt_eq(const Fpar_type* const a, const Fpar_type* const b) { return *a == *b; }

std::vector<condensed_fpar_list_item>* get_condensed_rep_of_fpars(const Fpar_def_list* const fpdl) {
	std::vector<condensed_fpar_list_item>* v = new std::vector<condensed_fpar_list_item>();
	if(fpdl != nullptr)
		for(const auto &fp : fpdl->item_list)
			v->push_back(condensed_fpar_list_item(fp->get_fpt(), fp->get_idlist_size()));
	return v;
}

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
