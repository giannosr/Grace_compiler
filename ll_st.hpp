#ifndef __LL_ST__
#define __LL_ST__

#include <map>
#include <vector>
#include <string>

/* which of these do we actually need??? */
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Transforms/Utils.h>

struct ll_ste {
  ll_ste(llvm::Value *val) : v(val) {}
  llvm::Value *v;
};

class ll_symbol_table {
  public:
    ll_symbol_table() : scopes() {}
    void push_scope() { scopes.push_back(std::map<std::string, ll_ste*>()); }
    void pop_scope()  { scopes.pop_back(); }
    void new_symbol(const char* const name, llvm::Value* v) {
      scopes.back().insert({std::string(name), new ll_ste(v)});
    }
    ll_ste* lookup(const std::string name, bool assigning) const {
      for(auto s = scopes.rbegin(); s != scopes.rend(); ++s) {
				auto i = s->find(name);
				if(i != s->end()) return i->second;
      }
      return nullptr;
    }
  private:
    std::vector<std::map<std::string, ll_ste*> > scopes;
};

#endif