#ifndef __SYMBOL_HPP__
#define __SYMBOL_HPP__

#include <iostream>
#include <vector>
#include <map>
#include <set>

// Do print debug tests                       [yes]
// Add library identifiers                    [yes]
// (move scope print to different file so types can be printed correctly?)
// Fix print of identifiers throught yyerror  []
//   -> in all files. Also remove lineno print in semantic errors because it's useless. also better style like [Semantic Erro]?
// Test the owed datastructure                [didn't but it works]
// Maybe refactor later                       []
// possibly seperate typ and fpar type?       [NO]
// possibly change sem to not have message and create 2 funs for header? [yes]

extern void yyerror(const char *msg);

class Type;
class Ret_type;
class Fpar_type;
class Fpar_def_list;

extern Ret_type rNothing;
extern Ret_type rInt;
extern Ret_type rChar;

/* Runtime Library Functions */
extern Fpar_def_list writeInteger_pars;
extern Fpar_def_list writeChar_pars;
extern Fpar_def_list writeString_pars;
/* readInteger has no formal pars so a nullptr will be used in symbol construction */
/* readChar    has no formal pars so a nullptr will be used in symbol construction */
extern Fpar_def_list readString_pars;
extern Fpar_def_list ascii_pars;
extern Fpar_def_list chr_pars;
extern Fpar_def_list strlen_pars;
extern Fpar_def_list strcmp_pars;
extern Fpar_def_list strcpy_pars;
extern Fpar_def_list strcat_pars;

extern void finish_runtime_syms();

extern bool check_fpt_eq(const Fpar_type* const a, const Fpar_type* const b);

struct condensed_fpar_list_item {
	condensed_fpar_list_item(Fpar_type* f, unsigned long long num) : fpt(f), n(num) {}
	Fpar_type* fpt;
	unsigned long long n;
	bool operator==(const condensed_fpar_list_item &c) const {
		return check_fpt_eq(fpt, c.fpt) && n == c.n;
	}
};

extern std::vector<condensed_fpar_list_item>* get_condensed_rep_of_fpars(const Fpar_def_list* const fpdl);

struct stentry {
	stentry(bool is_f, Type* const ty, const Ret_type* const rty=nullptr, const std::vector<condensed_fpar_list_item>* const fp=nullptr) : is_fun(is_f), t(ty), rt(rty), fpars(fp) {}
	bool is_fun;
	Type* const t;
	const Ret_type* const rt;
	const std::vector<condensed_fpar_list_item>* const fpars;
};

class scope {
	public:
		scope() {}
		stentry* lookup(const char* const id_name) {
			std::string s(id_name);
			if(symbols.find(s) == symbols.end()) return nullptr;
			return symbols[s];
		}

		void new_symbol(const char* const identifier_name, bool is_fun, Type* const t, const Ret_type* const rt=nullptr, const Fpar_def_list* const fpdl=nullptr, bool is_fdecl=false) {
			if(is_fun && rt == nullptr || !is_fun && t == nullptr) {
				yyerror("Compiler Bug: you fucked up");
				return;
			}

			std::string id_name(identifier_name);

			auto e = symbols.find(id_name);
			if(e != symbols.end()) {
				if(is_fun && owed.find(id_name) != owed.end()) {
					std::vector<condensed_fpar_list_item>* v = get_condensed_rep_of_fpars(fpdl);
					if(*v != *(e->second->fpars)) {
						yyerror("Semantic Error: identifier was previously declared with different formal parameters: ");
						std::cout << id_name << std::endl;
						return;
					}

					latest = new stentry(is_fun, nullptr, rt, v);
					owed.erase(id_name);
					return;
				}

				yyerror("Semantic Error: redeclaration of identifier: ");
				std::cout << id_name << std::endl;
			}

			if(is_fun) {
				std::vector<condensed_fpar_list_item>* v = get_condensed_rep_of_fpars(fpdl);
				if(is_fdecl) owed.insert(id_name);
				symbols[id_name] = latest = new stentry(is_fun, nullptr, rt, v);
			}
			else { // variable
				symbols[id_name] = latest = new stentry(is_fun, t); // is_fun is false so it is_not_a_fun
			}
		}

		stentry *get_latest() { return latest; }

		bool owes() { return !owed.empty(); }
		std::set<std::string> owed;
	//private:
		std::map<std::string, stentry*> symbols;
		stentry *latest;
};

class symbol_table {
	public:
		symbol_table() : scopes() {
			finish_runtime_syms(); // CAUTION, CALL SYMBOL TABLE CONSTRUCTOR ONLY ONCE,
								   // OTHERWISE IT WILL APPEND RUNTIME LIB FORMAL PARAMETERS AGAIN FOR SOME FUNS (because of this function, we could make it not do that)
			
			push_scope();
			new_symbol("writeInteger", true, nullptr, &rNothing, &writeInteger_pars);
			new_symbol("writeChar",    true, nullptr, &rNothing, &writeChar_pars);
			new_symbol("writeString",  true, nullptr, &rNothing, &writeString_pars);

			new_symbol("readInteger",  true, nullptr, &rInt,     nullptr);
			new_symbol("readChar",     true, nullptr, &rChar,    nullptr);
			new_symbol("readString",   true, nullptr, &rNothing, &readString_pars);

			new_symbol("ascii",        true, nullptr, &rInt,     &ascii_pars);
			new_symbol("chr",          true, nullptr, &rChar,    &chr_pars);

			new_symbol("strlen",       true, nullptr, &rInt,     &strlen_pars);
			new_symbol("strcmp",       true, nullptr, &rInt,     &strcmp_pars);
			new_symbol("strcpy",       true, nullptr, &rNothing, &strcpy_pars);
			new_symbol("strcat",       true, nullptr, &rNothing, &strcat_pars);
			// the first identifier "main" will also be placed in this scope and set as the owner of the next scope because of the way the compiler uses the symbol table (see ast.hpp in Header class)
		}
		~symbol_table() { while(!scopes.empty()) pop_scope(); }
		stentry* lookup(const char* const id_name) {
			for(auto s = scopes.rbegin(); s != scopes.rend(); ++s) {
				stentry *e = s->lookup(id_name);
				if(e != nullptr) return e;
			}
			std::cout << id_name << std::endl;
			yyerror("Semantic Error: Usage of undeclared identifier: ");
			return nullptr;
		}

		void new_symbol(const char* const id_name, bool is_fun, Type* const t, const Ret_type* const rt=nullptr, const Fpar_def_list* const fpdl=nullptr, bool is_fdecl=false) {
			scopes.back().new_symbol(id_name, is_fun, t, rt, fpdl, is_fdecl);
		}

		void push_scope() { scopes.push_back(scope()); }
		void pop_scope() {
			/* MOVE PRINT CODE TO EXTERN AND ABOVE MAIN IN PARSER.Y SO THAT IT CAN PRINT 
			 * RETURN AND FPAR TYPES CORRECTLY
			 */
			/*
			// for DEBUG: impl a print to print everything before scope is popped
			// (we can then check if all symbols are correct)
			std::cout << "----------scope-----------" << std::endl;
			for(const auto &it : scopes.back().symbols) {
				std::cout << "Identifier:\t" << it.first << std::endl
				          << "Is function:\t" << it.second->is_fun << std::endl;
				if(it.second->is_fun) {
					std::cout << "Return Type:\n" << it.second->rt << std::endl
					          << "Formal Params:\n";
					// debug->print if it second fpars is empty
					for(auto const &fp : *(it.second->fpars))
						std::cout << fp.n << " of type" << std::endl
						          << fp.fpt << std::endl;
				}
				else
					std::cout << "Type:\n" << it.second->t << std::endl;
				std::cout << "+++++++++++++++++" << std::endl;
			}
			// end DEBUG
			*/

			if(scopes.back().owes()) {
				yyerror("Semantic Error: No definition provided in the same scope for declarations: ");
				for(const auto &it : scopes.back().owed)
					std::cout << it << ' ';
				std::cout << std::endl;
			}
			scopes.pop_back();
			scope_owners.pop_back();
		}

		const Ret_type *get_scope_owner_rtype() { return scope_owners.back()->rt; };
		void set_next_scope_owner_latest_symbol() { scope_owners.push_back(scopes.back().get_latest()); }
	private:
		std::vector<scope>    scopes;
		std::vector<stentry*> scope_owners;
};


#endif
