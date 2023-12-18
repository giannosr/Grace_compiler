.PHONY: clean distclean default

LLVMCONFIG=llvm-config

CC=g++
CXXFLAGS=-Wall -Wextra -Wpedantic -Wno-parentheses -Wno-dangling-else -g `$(LLVMCONFIG) --cxxflags`
LDFLAGS=`$(LLVMCONFIG) --ldflags --system-libs --libs all`

default: grc

lexer.cpp: lexer.l parser.hpp
	flex -s -o lexer.cpp lexer.l

lexer.o: lexer.cpp lexer.hpp parser.hpp ast.hpp ast.cpp symbol_table.hpp runtime_syms.cpp ll_st.hpp

parser.hpp parser.cpp: parser.y
	bison -dv -o parser.cpp parser.y

parser.o: parser.cpp parser.hpp lexer.hpp ast.hpp ast.cpp symbol_table.hpp runtime_syms.cpp ll_st.hpp

grc: lexer.o parser.o ast.o
	$(CC) $(CXXFLAGS) -o grc $^ $(LDFLAGS)

clean:
	$(RM) lexer.cpp parser.cpp parser.hpp parser.output lexer.o parser.o ast.o

distclean: clean
	$(RM) grc
