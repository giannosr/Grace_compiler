#ifndef __AST_HPP__
#define __AST_HPP__

#include <iostream>
#include <vector>

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
		virtual void print(std::ostream &out) const = 0;
};

inline std::ostream &operator<<(std::ostream &out, const AST &ast) {
	ast.print(out);
	return out;
}

/* Utils */

class Listable : public AST {
	public:
};

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
	private:
		std::vector<Listable*> item_list;
		const char* list_name;
};

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
		Data_type(const char* dtype) : data_type_name(dtype) {}
		void print(std::ostream &out) const override {
			align.begin(out, "Data Type");
			align.no_line();
			out << align << data_type_name << std::endl;
			align.end(out);
		}
	private:
		const char* data_type_name;
};

class Array_tail_decl : public AST {
	public:
		Array_tail_decl(unsigned long long n) : sizes(1, n) {}
		void append(unsigned long long n) { sizes.push_back(n); } // possiblle semantic analysis point: n > 0
		void print(std::ostream &out) const override {
			align.begin(out, "Array of size", true);
			for(const auto &n : sizes)
				out << '[' << n << ']';
			out << std::endl;
			align.end(out);
		}
	private:
		std::vector<unsigned long long> sizes;
};

class Type : public AST {
	public:
		Type(Data_type* dtype, Array_tail_decl* atdecl=nullptr) : dt(dtype), atd(atdecl) {}
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
	
	private:
		Data_type       *dt;
		Array_tail_decl *atd;
};

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
	private:
		Id_list *Identifier_list;
		Type *of_type;
};

class Func_decl : public Local_def {
	public: // maybe print that we are inside a function declaration
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
	private:
		Data_type *dt;
		bool      nothing;
};

class Fpar_type : public AST {
	public:
		Fpar_type(Data_type* data_ty, bool empty_array_layer, Array_tail_decl *atde) : dt(data_ty), no_size_array(empty_array_layer), atd(atde) {}
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
	private:
		Data_type       *dt;
		bool            no_size_array;
		Array_tail_decl *atd;
};

class Fpar_def : public Listable {
	public:
		Fpar_def(bool refrence, Id_list *idlist, Fpar_type *fpar_ty) : ref(refrence), idl(idlist), fpt(fpar_ty) {}
		void print(std::ostream &out) const override {
			align.begin(out, "Formal Parameter Definition");
			out << align << " is ";
			if(ref) out << "NOT ";
			out << "a refrence" << std::endl;
			out << *idl;
			out << align << " of type:" << std::endl;
			align.no_line();
			out << *fpt;
			align.end(out);
		}
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
	private:
		Id            *name;
		Fpar_def_list *params;
		Ret_type      *rtype;
};

class Local_def_list : public Item_list {
	public:
		Local_def_list() : Item_list("Local Definition List") {}
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
		void append(Stmt *s) override {s_list.append(s); }
		void print(std::ostream &out) const override { s_list.print(out); }
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
	private:
		Header         *h;
		Local_def_list *ldl;
		Block          *b;
};

/* Expressions & Conditions */

class Expr : public Listable {
	public: // maybe print we are inside an expression
};

class Int_const : public Expr {
	public:
		Int_const(unsigned long long value) : val(value) {}
		void print(std::ostream &out) const override {
			align.begin(out, "Integer Constant", true);
			out << ' ' << val << std::endl;
			align.end(out);
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
	private:
		Expr *e;
};


#endif
