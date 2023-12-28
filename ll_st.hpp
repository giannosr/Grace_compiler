#ifndef __LL_ST__
#define __LL_ST__

#include <map>
#include <vector>
#include <string>

#include <llvm/IR/Value.h>

struct ll_ste {
  ll_ste(llvm::Value* const val, llvm::Type* type, llvm::Type* const base, const unsigned long long in_frame_number, const unsigned long long scope_number) : v(val), t(type), base_type(base), frame_no(in_frame_number), scope_no(scope_number) {}
  llvm::Value *v;
  llvm::Type  *t;
  llvm::Type  *base_type;
  unsigned long long frame_no, scope_no;
};

struct ll_scope {
  ll_scope(const char* const f_name, llvm::Function* const fun) : vars(), func_name(f_name), f(fun) {}
  std::map<std::string, ll_ste*> vars;
  const char* const func_name;
  llvm::Function* const f;
};

class ll_symbol_table {
 public:
  ll_symbol_table() : scopes() {}
  void push_scope(const char* const func_name, llvm::Function* const f) {
    scopes.push_back(new ll_scope(func_name, f));
  }
  void pop_scope() {
    for(const auto &s : scopes.back()->vars) delete s.second;
    delete scopes.back();
    scopes.pop_back();
  }
  void new_symbol(const std::string name, llvm::Value* const v, llvm::Type* const t, llvm::Type* base_type=nullptr, const unsigned long long frame_no=-1) {
    scopes.back()->vars[name] = new ll_ste(v, t, base_type, frame_no, scopes.size());
  }
  const ll_ste* lookup(const std::string name) const {
    for(auto s = scopes.rbegin(); s != scopes.rend(); ++s) {
      auto i = (**s).vars.find(name);
      if(i != (**s).vars.end()) return i->second;
    }
    return nullptr;
  }
  const ll_ste* lookup(const std::string name, const unsigned long long scope) const {
    return scopes[scope - 1]->vars.find(name)->second;
  }
  std::string get_scope_name(const std::string& sep) const {
    if(scopes.empty()) return std::string("");
    auto s = scopes.begin();
    std::string s_name = std::string((**s).func_name);
    while(++s != scopes.end()) s_name += sep + std::string((**s).func_name);
    return s_name;
  }
  void fill_in_stack_frame(std::vector<std::pair<std::string, ll_ste*> > &vars, std::vector<llvm::Type*> &types) const {
    // unsigned long long i = 0;
    for(auto &v : scopes.back()->vars) {
      vars.push_back(v);
      types.push_back(v.second->t);
      // v.second->frame_no = ++i; // because frame_no 0 is the frame pointer
      // (not needed because new entries are generated for every var after this)
    }
  }
  unsigned long long get_current_scope_no() const { return scopes.size(); }
 private:
  std::vector<ll_scope*> scopes;
};

#endif
