#ifndef __AST_HPP__
#define __AST_HPP__

#include <iostream>
#include <vector>
#include <deque>
#include <cstring>

#include "symbol_table.hpp"

extern symbol_table st;

/* Global object to control print indentation
 * Callee is expected to use it
 *
 * Example usage:
 * (In callee's print)
 * align.begin(out, "My name");
 * out << align << "something I wanna say" << std::endl
 *     << align << my_var << std::endl
 *     << *other_callee1 << *other_callee2; // callees take care of their own alignment
 * align.no_line(); // optional so that the line stops drawing after the last child
 * out << *last_callee;
 * align.end(out);
 */
class print_align {
	public:
		print_align() : indent_num(0), line(), marked() {}
		void print(std::ostream &out) {
			for(int i = 0; i < indent_num; ++i)
				if(marked[i])    { out << "|-"; marked[i] = false; }
				else if(line[i]) { out << "| "; }
				else             { out << "  "; }
		}
		void indent()   { ++indent_num; line.push_back(true); marked.push_back(false); }
		void unindent() { --indent_num; line.pop_back(); marked.pop_back(); }
		void no_line()  { line[indent_num-1] = false; marked[indent_num-1] = true; }
		void begin(std::ostream &out, const char* const s, bool no_new_line=false) {
			if(indent_num > 0) marked[indent_num-1] = true;
			this->print(out);
			out << s;
			if(!no_new_line) out << std::endl;
			this->indent();
		}
		void end(std::ostream &out, bool no_new_line=false) {
			this->unindent();
			if(line[indent_num-1] && !no_new_line) {
				this->print(out);
				out << std::endl;
			}
		}
	private:
		int indent_num;
		std::vector<bool> line;
		std::vector<bool> marked;
};

inline std::ostream &operator<<(std::ostream &out, print_align &a) {
	a.print(out);
	return out;
}

extern print_align align;

/* end of print format code, AST code follows */


class AST {
	public:
		virtual void sem() {};
		virtual void print(std::ostream &out) const = 0;
};

inline std::ostream &operator<<(std::ostream &out, const AST &ast) {
	ast.print(out);
	return out;
}

/* Warning: Some Bad Code Ahead
 * (it was impossible to do some of the dirty work cleanly)
 * (particularly for semantic analysis)
 * Also due to semantic analysis it's possible there are memory leaks
 * because I was too lazy to write destructors for every class (so we
 * rely on the default destructors which should be fine). However if
 * someone were to write the destructors then there wouldn't be any
 * memory leaks. But it shouldn't matter in a program like this which
 * is guaranteed to terminate quickly.
 * Inheritance was a bad idea and is responsible for much of the bad code
 */

/* Utils */
class Fpar_type;
class Listable : public AST { // this was a really bad idea but it can't be changed now
	public:
		virtual const char* get_name() const { return 0; }
		virtual unsigned long long get_idlist_size() const { return 0; }
		virtual Fpar_type* get_fpt() const { return 0; }
		virtual bool check_comp_with_fpt(Fpar_type* fpt) const { return 0; } // this is bad. Should I return *nullptr instead?
}; // lol these funs are for specific types of listables but the way iterators work means we have to declare them here (all are for semantic analysis btw)
 
class Item_list : public AST {
	public:
		Item_list(const char* name) : item_list(), list_name(name) {}
		void append(Listable* item) { item_list.push_back(item); }
		void print(std::ostream &out) const override {
			align.begin(out, list_name, true);
			if(item_list.size() == 0) {
				out << " (empty)" << std::endl;
				align.end(out);
				return;
			}
			out << std::endl;
			for(const auto &item : item_list)
				if(item != item_list.back()) out << *item;
			align.no_line();
			out << *item_list.back();
			align.end(out);
		}
		
		std::vector<Listable*> item_list;
	private:
		const char* list_name;
};

/* place holder class for incomplete parts of the compiler, not in use */
class Under_construction : public Listable {
	public:
		Under_construction(const char* s) : name(s) {}
		void print(std::ostream &out) const override {
			align.begin(out, name);
			out << align << "Under" << std::endl
			    << align << "construction" << std::endl;
			align.end(out);
		}
	private:
		const char *name;
};

/* End of Utils, Actual Syntax Structures */

class Id : public Listable {
	public:
		Id(const char* id_name) : name(id_name) {}
		void print(std::ostream &out) const override {
			align.begin(out, "Identifier", true);
			out << " " << name << std::endl;
			align.end(out, true);
		}

		const char* get_name() const override { return name; }
	private:
		const char* name;
};

class Id_list : public Item_list {
	public:
		Id_list(Listable* id) : Item_list("Identifier List") {
			this->append(id);
		}
};

class Data_type : public AST {
	public:
		Data_type(const char* dtype) : data_type_name(dtype), cleanup(false) {}
		Data_type(const Data_type &dt) : data_type_name(strdup(dt.data_type_name)), cleanup(true) {}
		~Data_type() { if(cleanup) delete data_type_name; }
		void print(std::ostream &out) const override {
			align.begin(out, "Data Type");
			align.no_line();
			out << align << data_type_name << std::endl;
			align.end(out);
		}

		const char* get_dt_name() const { return data_type_name; }
		bool operator==(const Data_type &dt) const {
			return !strcmp(data_type_name, dt.get_dt_name());
		}
	private:
		const char* data_type_name;
		bool cleanup;
};

class Array_tail_decl : public AST {
	public:
		Array_tail_decl(unsigned long long n) : sizes(1, n) {}
		Array_tail_decl(const Array_tail_decl &atd) : sizes(atd.sizes) {}
		void append(unsigned long long n) { sizes.push_back(n); } // possiblle semantic analysis point: n > 0
		void print(std::ostream &out) const override {
			align.begin(out, "Array of size", true);
			for(const auto &n : sizes)
				out << '[' << n << ']';
			out << std::endl;
			align.end(out);
		}

		void sem() {
			for(const auto &n : sizes)
				if(n == 0) {
					std::cout << *this;
					yyerror("Semantic Error: array sizes cannot be 0");
				}
		}

		bool operator==(const Array_tail_decl &atd) const {
			return sizes == atd.sizes;
		}

		// no private because makes above == operator easier lol
		std::deque<unsigned long long> sizes;
};

class Type : public AST {
	public:
		Type(Data_type* dtype, Array_tail_decl* atdecl=nullptr) : dt(dtype), atd(atdecl) {}
		Type(const Type &t) : dt(new Data_type(*(t.dt))), atd(t.atd == nullptr ? nullptr : new Array_tail_decl(*(t.atd))) {}
		// may require destructor
		void print(std::ostream &out) const override {
			align.begin(out, "Type");
			if(atd == nullptr) {
				align.no_line();
				out << *dt;
			}
			else {
				out << *dt;
				align.no_line();
				out << *atd;
			}
			align.end(out);
		}

		bool operator==(const Type &t) const {
			/* Complex Logic Follows (maybe atd shouldn't have been allowed to be null but instead hold an empty vector) */
			bool r1 = (*dt == *(t.dt));
			if(atd != nullptr && t.atd != nullptr)
				r1 = r1 && (*atd == *(t.atd));
			else if(atd != nullptr || t.atd != nullptr)
				return false;
			return r1;
		}

		void drop_last() { // only use this on copied types because it modifies the object
			if(atd == nullptr)
				yyerror("Compiler bug: you tried to delete array part of non array type");
			atd->sizes.pop_back();
			if(atd->sizes.empty()) {
				delete atd;
				atd = nullptr;
			}
		}

		void drop_first() { // only use this on copied. Usefull for fpar type checking
			if(atd == nullptr)
				yyerror("Compiler bug: you tried to delete array part of non array type (front)");
			atd->sizes.pop_front();
			if(atd->sizes.empty()) {
				delete atd;
				atd = nullptr;
			}
		}

		void sem() {
			if(atd != nullptr) atd->sem();
		}

		// no private because it's easier for inheritance
		// I could do protected or something but I don't remember how it works exactly
		// and I can't be bothered to look it up.
		// (later) Also I used the fact they are public elsewhere
		// so protected would break it now lol
		Data_type       *dt;
		Array_tail_decl *atd;
};

/* some usefull types */
extern const char *INT, *CHAR;

extern Type Int_t;
extern Type Char_t;

class Str_type : public Type {
	public:
		Str_type(unsigned long long size) : Type(new Data_type(CHAR), new Array_tail_decl(size)) {}
};

/* more AST structures */

class Local_def : public Listable {
	public:
};

class Var_def : public Local_def {
	public:
		Var_def(Id_list* idl, Type* t) : Identifier_list(idl), of_type(t) {}
		void print(std::ostream &out) const override {
			align.begin(out, "Variable Definition");
			out << *Identifier_list
			    << align << " All above are of type:"<< std::endl;
			align.no_line();
			out << *of_type;
			align.end(out);
		}

		void sem() {
			for(const auto &id : Identifier_list->item_list)
				st.new_symbol(id->get_name(), false, of_type);
			// Here make sure n>0 in array def and NOT in the prev point
			// check type
			of_type->sem();
		}
	private:
		Id_list *Identifier_list;
		Type *of_type;
};

class Func_decl : public Local_def {
	public: // maybe print that we are inside a function declaration
};

class Fpar_type : public Type {
	public:
		Fpar_type(Data_type* data_ty, bool empty_array_layer, Array_tail_decl *atde) : Type(data_ty, atde), no_size_array(empty_array_layer) {}
		void print(std::ostream &out) const override {
			align.begin(out, "Formal Parameter Type");
			if(!no_size_array and atd == nullptr) {
				align.no_line();
				out << *dt;
				align.end(out);
				return;
			}
			out << *dt;
			if(atd == nullptr)
				align.no_line();
			if(no_size_array)
				out << align << "[]" << std::endl; // find a better way to express this?
			if(atd != nullptr) {
				align.no_line();
				out << *atd;
			}
			align.end(out);
		}

		bool operator==(const Fpar_type &f) const {
			return no_size_array == f.no_size_array && dt == f.dt && atd == f.atd;
		}

		bool has_unk_size_arr() { return no_size_array; }
		bool is_array() { return no_size_array || (atd != nullptr); }
		Type* to_type() {
			if(atd == nullptr)
				if(no_size_array) return new Type(new Data_type(*dt), new Array_tail_decl(1));
				else              return new Type(new Data_type(*dt));
			Type *t = new Type(new Data_type(*dt), new Array_tail_decl(*atd));
			if(no_size_array)
				t->atd->sizes.push_front(1); // fuck the rules
			return t;
		} 
		bool is_comp_with_t(const Type* const t) {
			if(no_size_array) {
				Type *p = new Type(*t); // we are gonna modify it so we copy
				p->drop_first();
				bool res = private_t_check(p);
				delete p;
				return res;
			}
			return private_t_check(t);
		}
	private:
		bool no_size_array;
		// other vars are inherited by Type
		bool private_t_check(const Type* const t) {
			/* Complex Logic Follows (maybe atd shouldn't have been allowed to be null but instead hold an empty vector) */
			bool r1 = (*dt == *(t->dt));
			if(atd != nullptr && t->atd != nullptr)
				r1 = r1 && (*atd == *(t->atd));
			else if(atd != nullptr || t->atd != nullptr)
				return false;
			return r1;
		}
};

class Fpar_def : public Listable {
	public:
		Fpar_def(bool is_ref, Id_list *idlist, Fpar_type *fpar_ty) : ref(is_ref), idl(idlist), fpt(fpar_ty) {}
		void print(std::ostream &out) const override {
			align.begin(out, "Formal Parameter Definition");	
			if(ref) out << align << "BY REF" << std::endl;
			out << *idl;
			out << align << " of type:" << std::endl;
			align.no_line();
			out << *fpt;
			align.end(out);
		}

		void sem() {
			if(fpt->is_array() && !ref)
				yyerror("Semantic Error: array types can only be passed by reference to functions");
			for(const auto &id : idl->item_list) st.new_symbol(id->get_name(), false, fpt->to_type());
			// because of this line and particuarly the call to_type() if the symbol table is destroyed before the end of the program there will be a memory leak
			// It is necessary because the rest of the program needs the stentry to contain a type, not a formal type
		}

		unsigned long long get_idlist_size() const override { return idl->item_list.size(); }
		Fpar_type* get_fpt() const override { return fpt; }
	private:
		bool      ref;
		Id_list   *idl;
		Fpar_type *fpt;
};

class Fpar_def_list : public Item_list {
	public:
		Fpar_def_list(Fpar_def* fpd) : Item_list("Formal Parameter Definition List") {
			this->append(fpd);
		}
};

class Ret_type : public AST {
	public:
		Ret_type(Data_type* data_ty) : dt(data_ty), nothing(data_ty == nullptr) {}
		void print(std::ostream &out) const override {
			align.begin(out, "Return Type");
			align.no_line();
			if(nothing) out << align << "nothing" << std::endl;
			else        out << *dt;
			align.end(out);
		}

		bool check_eq_with_t(Type* t) const {
			if(nothing || t->atd != nullptr) return false;
			return *dt == *(t->dt);
		}
		
		bool check_comp_with_fpt(Fpar_type* fpt) const {
			if(nothing || fpt->atd != nullptr || fpt->has_unk_size_arr()) return false;
			return *dt == *(fpt->dt); // oh no it violates oop. trust me this is better. otherwise we have to write a lot of extra code to either do the same thing or something much worse using dynamic alloactions
		}

		bool is_nothing() const { return nothing; }
	private:
		Data_type *dt;
		bool      nothing;
};

class Header : public Func_decl {
	public:
		Header(Id* id, Fpar_def_list* parameters, Ret_type* return_type) : name(id), params(parameters), rtype(return_type) {}
		void print(std::ostream &out) const override {
			align.begin(out, "Header");
			out << *name;
			out << align << std::endl;
			if(params != nullptr) out << *params;
			align.no_line();
			out << *rtype;
			align.end(out);
		}

		void sem() { st.new_symbol(name->get_name(), true, nullptr, rtype, params, true); } // this is called by Func_decl so it is always a declaration
		void semdef() { // this is only called by Func_def
			st.new_symbol(name->get_name(), true, nullptr, rtype, params); // maybe convert rtype to str and params to condensed vector here?? How will this affect ret type checking in sem later??
			st.set_next_scope_owner_latest_symbol();
			st.push_scope(); // will be popped by caller
			// new symbols for all new parameters in new scope
			if(params != nullptr)
				for(const auto &fpd : params->item_list)
					fpd->sem();
		}
	private:
		Id            *name;
		Fpar_def_list *params;
		Ret_type      *rtype;
};

class Local_def_list : public Item_list {
	public:
		Local_def_list() : Item_list("Local Definition List") {}

		void sem() {
			for(const auto &it : item_list) it->sem();
		}
};

class Stmt : public Listable {
	public: // maybe print we are inside a stmt
};

class Block : public Stmt {
	public:
		virtual void append(Stmt *s) = 0;
		// maybe print we are inside a block
};

class Stmt_list : public Block {
	public:
		Stmt_list() : s_list("Statement List") {}
		void append(Stmt *s) override { s_list.append(s); }
		void print(std::ostream &out) const override { s_list.print(out); }
		void sem() {
			for(auto const &s : s_list.item_list) s->sem();
		}
	private:
		Item_list s_list;
};

class Func_def : public Local_def {
	public:
		Func_def(Header *he, Local_def_list *ldli, Block *bl) : h(he), ldl(ldli), b(bl) {}
		void print(std::ostream &out) const override {
			align.begin(out, "Function Definition");
			out << *h << *ldl;
			align.no_line();
			out << *b;
			align.end(out);
		}

		void sem() {
			h->semdef(); // pushes a scope because we are in a function def
			ldl->sem();
			b->sem();
			st.pop_scope();
		}
	private:
		Header         *h;
		Local_def_list *ldl;
		Block          *b;
};

/* Expressions & Conditions */

class Expr : public Listable {
	public: // maybe print we are inside an expression
		virtual bool check_type(Type* t) = 0;
		virtual bool check_comp_with_fpt(Fpar_type* fpt) const = 0;
};

class Int_const : public Expr {
	public:
		Int_const(unsigned long long value) : val(value) {}
		void print(std::ostream &out) const override {
			align.begin(out, "Integer Constant", true);
			out << ' ' << val << std::endl;
			align.end(out);
		}

		bool check_type(Type* t) override {
			return *t == Int_t;
		}

		bool check_comp_with_fpt(Fpar_type* fpt) const override {
			return fpt->is_comp_with_t(&Int_t);
		}
	private:
		unsigned long long val;
};

class Char_const : public Expr { // char is treated as a string. maybe evaluate it?
	public:
		Char_const(const char* chr) : ch(chr) {}
		void print(std::ostream &out) const override {
			align.begin(out, "Character Constant", true);
			out << ' ' << ch << std::endl;
			align.end(out);
		}

		bool check_type(Type* t) override {
			return *t == Char_t;
		}
		
		bool check_comp_with_fpt(Fpar_type* fpt) const override {
			return fpt->is_comp_with_t(&Char_t);
		}
	private:
		const char *ch;
};

/* Func_call is also an expression but is a statement too so it's defined later */

class UnOp : public Expr {
	public:
		UnOp(char Operator, Expr* exp) : op(Operator), e(exp) {}
		void print(std::ostream &out) const override {
			align.begin(out, "Unary Operation");
			out << align << "Op(" << op << ')' << std::endl;
			align.no_line();
			out << *e;
			align.end(out);
		}

		void sem() {
			if(!e->check_type(&Int_t))
				yyerror("Semantic Error: Operant of unary operator must be of type int");
		}

		bool check_type(Type* t) override {
			sem();
			return *t == Int_t;
		}
		
		bool check_comp_with_fpt(Fpar_type* fpt) const override {
			return fpt->is_comp_with_t(&Int_t);
		}
	private:
		char op;
		Expr *e;
};

#define AND_OP 'a'
#define DIV_OP 'd'
#define MOD_OP 'm'
#define OR_OP  'o'
#define LEQ_OP 'l'
#define GEQ_OP 'g'
class BinOp : public Expr {
	public:
		BinOp(Expr* left, char Operator, Expr* right) : l(left), op(Operator), r(right) {}
		void print(std::ostream &out) const override {
			align.begin(out, "Binary Operation");
			out << *l
			    << align << "Op(" << op << ')' << std::endl
			    << align << std::endl;
			align.no_line();
		    out << *r;
			align.end(out);
		}

		void sem() {
			if(!l->check_type(&Int_t)) {
				yyerror("Sematnic Error: left argument of binary operator must be of type int. op was ");
				std::cout << op;
			}
			if(!r->check_type(&Int_t)) {
				yyerror("Sematnic Error: right argument of binary operator must be of type int. op was ");
				std::cout << op;
			}
		}

		bool check_type(Type* t) override {
			sem();
			return *t == Int_t;
		}
		
		bool check_comp_with_fpt(Fpar_type* fpt) const override {
			return fpt->is_comp_with_t(&Int_t);
		}
	private:
		Expr *l;
		char op;
		Expr *r;
};

class Expr_list : public Item_list {
	public:
		Expr_list(Expr* expr) : Item_list("Expression List") {
			this->append(expr);
		}

		void sem() {
			for(auto const &expr : item_list) expr->sem();
		}
};

class Cond : public AST {
	public:
};

class NotCond : public Cond {
	public:
		NotCond(Cond* cond) : c(cond) {}
		void print(std::ostream &out) const override {
			align.begin(out, "Not Condtion");
			out << align << "OP(not)" << std::endl;
			align.no_line();
			out << *c;
			align.end(out);
		}

		void sem() {
			c->sem();
		}
	private:
		Cond *c;
};

class BinCond : public Cond {
	public:
		BinCond(Cond* left, char Operator, Cond* right) : l(left), op(Operator), r(right) {}
		void print(std::ostream &out) const override {
			align.begin(out, "Binary Condition");
			out << *l
			    << align << "Op(" << op << ')' << std::endl
			    << align << std::endl;
			align.no_line();
			out << *r;
			align.end(out);
		}
		
		void sem() {
			l->sem();
			r->sem();
		}
	private:
		Cond *l;
		char op;
		Cond *r;
};

class BinOpCond : public Cond {
	public:
		BinOpCond(Expr* left, char Operator, Expr* right) : l(left), op(Operator), r(right) {}
		void print(std::ostream &out) const override {
			align.begin(out, "Binary Operational Condition");
			out << *l
			    << align << "Op(" << op << ')' << std::endl
			    << align << std::endl;
			align.no_line();
		    out << *r;
			align.end(out);
		}

		void sem() {
			bool valid = l->check_type(&Int_t) && r->check_type(&Int_t)
			          || l->check_type(&Char_t) && r->check_type(&Char_t);
			if(!valid) {
				std::cout << *this;
				yyerror("Semantic Error: comparison between different types");
			}
		}
	private:
		Expr *l;
		char op;
		Expr *r;

};

class L_value : public Expr {
	public:
		L_value(Id* identifier, const char* str_literal, L_value* l_value, Expr* expr) : id(identifier), str(str_literal), lv(l_value), e(expr) {}
		void print(std::ostream &out) const override {
			align.begin(out, "L Value");
			if(lv != nullptr)
				out << *lv
				    << align << " [" << std::endl;
			align.no_line();
			if(e != nullptr)
				out << *e
				    << align << " ]" << std::endl;
			else if(id != nullptr)
				out << *id;
			else
				out << align << str << std::endl;;
			align.end(out);
		}

		Type* get_type(bool &del_after) const {
			if(id != nullptr) {
				del_after = false;
				stentry *ste = st.lookup(id->get_name());
				if(ste->t == nullptr) {
					std::cout << *id;
					yyerror("Sematnic error: this identifier belongs to a function not an lvalue (did you forget to put parenthesis?)");
				}
				return ste->t;
			}
			if(str != nullptr) { del_after = true; return new Str_type(strlen(str)); } // to prevent memory leak
			if(!e->check_type(&Int_t))
				yyerror("Semantic Error: Access Expression must be of type int");
			Type *p = lv->get_type(del_after);
			Type *t = del_after ? p : new Type(*p);
			del_after = true;
			t->drop_last();
			return t;
		}

		bool check_type(Type* t) override {
			bool del_after;
			Type *my_t = get_type(del_after);
			bool res = (*t == *my_t);
			if(del_after) delete my_t;
			return res;
		}
		
		bool check_comp_with_fpt(Fpar_type* fpt) const override {
			bool del_after, res;
			Type *t = get_type(del_after);
			res = fpt->is_comp_with_t(t);
			if(del_after) delete t;
			return res;
		}
	private:
		Id         *id;
		const char *str;
		L_value    *lv;
		Expr       *e;
};

/* Statements */

class Empty_stmt : public Stmt {
	public:
		Empty_stmt() {}
		void print(std::ostream &out) const override {
			align.begin(out, "; (empty statement)");
			align.end(out);
		}
};

class Assign : public Stmt {
	public:
		Assign(L_value* l_value, Expr* expr) : lv(l_value), e(expr) {}
		void print(std::ostream &out) const override {
			align.begin(out, "Assign Statement");
			out << *lv
			    << align << " <-" << std::endl;
			align.no_line();
			out	<< *e;
			align.end(out);
		}

		void sem() {
			bool del_after;
			Type *t = lv->get_type(del_after);
			if(t->atd != nullptr)
				yyerror("Semantic Error: Assignement to and from array types is not allowed");
			if(!e->check_type(t)) {
				std::cout << *lv << *t << *e;
				yyerror("Semantic Error: Trying to assign expression to lvalue of different type. lvalue is of type: ");
			}
			if(del_after) delete t;
		}
	private:
		L_value *lv;
		Expr    *e;
};

class Func_call : public Stmt, public Expr {
	public:
		Func_call(Id* identifier, Expr_list* exp_list) : id(identifier), e_list(exp_list) {}
		void print(std::ostream &out) const override {
			align.begin(out, "Function Call");
			if(e_list == nullptr) align.no_line(); // factor ifs better?
			out << *id
			    << align << " ()" << std::endl;
			if(e_list != nullptr) {
				align.no_line();
				out << *e_list;
			}
			align.end(out);
		}

		void sem() {
			stentry *e = st.lookup(id->get_name());
			if(e->rt == nullptr) {
				std::cout << *id;
				yyerror("Semantic error: this identifier belongs to an lvalue not a function (did you accidentally put parenthesis?)");
			}
			if(e_list == nullptr) {
				if(!e->fpars->empty()) {
					yyerror("Semantic Error: formal parameter missmatch in function call. No paramters given when function expects formal parameters");
					std::cout << id->get_name() << std::endl;
				}

				return;
			}
			e_list->sem();
			auto it = e_list->item_list.begin();
			for(auto const &fp : *(e->fpars))
				for(int i = 0; i < fp.n; ++i) {
					if(it == e_list->item_list.end())
						yyerror("Semantic Error: formal parameter missmatch in function call. Less parameters supplied");
					if(!(*it)->check_comp_with_fpt(fp.fpt)) {
						std::cout << *(fp.fpt) << **it;
						yyerror("Semantic Error: formal parameter type missmatch in function call. Formal parameter is ");
					}
					++it;
				}
			if(it != e_list->item_list.end())
				yyerror("Semantic Error: formal parameter missmatch in function call. More parameters given than accepted by function");
		}

		bool check_type(Type* t) override {
			sem();
			return st.lookup(id->get_name())->rt->check_eq_with_t(t);
		}
		
		bool check_comp_with_fpt(Fpar_type* fpt) const override {
			return st.lookup(id->get_name())->rt->check_comp_with_fpt(fpt);
		}
	private:
		Id        *id;
		Expr_list *e_list;
};

class If : public Stmt {
	public:
		If(Cond* cond, Stmt* Then_stmt, Stmt* Else_stmt) : c(cond), Then(Then_stmt), Else(Else_stmt) {}
		void print(std::ostream &out) const override {
			align.begin(out, "If Statement");
			out << align << " IF" << std::endl
			    << *c
			    << align << " THEN" << std::endl;
			if(Else == nullptr)	align.no_line(); // better if factorization?
			out << *Then;
			if(Else != nullptr) {
				out << align << " ELSE" << std::endl;
				align.no_line();
				out << *Else;
			}
			align.end(out);
		}

		void sem() {
			c->sem();
			Then->sem();
			if(Else != nullptr) Else->sem();
		}
	private:
		Cond *c;
		Stmt *Then;
		Stmt *Else;
};

class While : public Stmt {
	public:
		While(Cond* cond, Stmt* stmt) : c(cond), s(stmt) {}
		void print(std::ostream &out) const override {
			align.begin(out, "While Statement");
			out << *c;
			align.no_line();
			out << *s;
			align.end(out);
		}

		void sem(void* sem) {
			c->sem();
			s->sem();
		}
	private:
		Cond *c;
		Stmt *s;
};

class Return : public Stmt {
	public:
		Return(Expr* expr) : e(expr) {}
		void print(std::ostream &out) const override {
			align.begin(out, "Return Statement");
			if(e != nullptr) {
				align.no_line();
				out << *e;
			}
			align.end(out);
		}

		void sem() {
			const Ret_type *rt = st.get_scope_owner_rtype();
			if(e == nullptr)
				if(rt->is_nothing())
					return;
				else
					yyerror("Semantic error: return expression lacks a return value when function return type is not nothing");
			else if(rt->is_nothing())
				yyerror("Semantic error: function return type is nothing but return expression contains something");
			
			if(rt->check_eq_with_t(&Int_t)  && e->check_type(&Int_t)
			|| rt->check_eq_with_t(&Char_t) && e->check_type(&Char_t))
				return;
			else
				yyerror("Semantic Error: Type missmatch in return statement, function has a different return type than that of returned expression");
	}
	private:
		Expr *e;
};


#endif
