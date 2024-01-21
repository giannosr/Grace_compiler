.PHONY: clean distclean default

LLVMCONFIG=llvm-config

CC=clang++
CXXFLAGS=-O2 `$(LLVMCONFIG) --cxxflags`
LDFLAGS=`$(LLVMCONFIG) --ldflags --system-libs --libs all`

default: grc libgrc/libgrc.a

lexer.cpp: lexer.l parser.hpp
	flex -s -o lexer.cpp lexer.l

lexer.o: lexer.cpp lexer.hpp parser.hpp ast.hpp ast.cpp symbol_table.hpp runtime_syms.cpp ll_st.hpp

parser.hpp parser.cpp: parser.y
	bison -dv -o parser.cpp parser.y

parser.o: parser.cpp parser.hpp lexer.hpp ast.hpp ast.cpp symbol_table.hpp runtime_syms.cpp ll_st.hpp

grc: lexer.o parser.o ast.o
	$(CC) $(CXXFLAGS) -o grc $^ $(LDFLAGS)
	chmod +x grc.py

libgrc/libgrc.a: libgrc/src
	cd libgrc; make; cd ..

clean:
	$(RM) lexer.cpp parser.cpp parser.hpp parser.output lexer.o parser.o ast.o

distclean: clean
	chmod -x grc.py
	$(RM) grc
	cd libgrc; make clean; cd ..
