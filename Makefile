.PHONY: clean distclean default

CC=g++
CFLAGS=-Wall -Wextra

default: grc

lexer.cpp: lexer.l parser.hpp
	flex -s -o lexer.cpp lexer.l

lexer.o: lexer.cpp lexer.hpp parser.hpp ast.hpp ast.cpp symbol_table.hpp runtime_syms.cpp ll_st.hpp

parser.hpp parser.cpp: parser.y
	bison -dv -o parser.cpp parser.y

parser.o: parser.cpp parser.hpp lexer.hpp ast.hpp ast.cpp symbol_table.hpp runtime_syms.cpp ll_st.hpp

grc: lexer.o parser.o
	$(CC) $(CFLAGS) -o grc lexer.o parser.o

clean:
	$(RM) lexer.cpp parser.cpp parser.hpp parser.output lexer.o parser.o

distclean: clean
	$(RM) grc
