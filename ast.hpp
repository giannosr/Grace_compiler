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
		Array_tail_decl(int n) : sizes(1, n) {}
		void append(int n) { sizes.push_back(n); } // possiblle semantic analysis point: n > 0
		void print(std::ostream &out) const override {
			align.begin(out, "Array of size", true);
			for(const auto &n : sizes)
				out << '[' << n << ']';
			out << std::endl;
			align.end(out);
		}
	private:
		std::vector<int> sizes;
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
		Data_type *dt;
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
		bool       nothing;
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

class Block : public AST {
	public:
		Block() {} // fill in later
		void print(std::ostream &out) const override {
			align.begin(out, "Block");
			out << align << "fill" << std::endl
				<< align << "in" << std::endl
				<< align << "later" << std::endl;
			align.end(out);
		}
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
		Header *h;
		Local_def_list *ldl;
		Block *b;
};


#endif
