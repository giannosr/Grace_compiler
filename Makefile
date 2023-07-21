.PHONY: clean distclean default

CC=g++
CFLAGS=-Wall

default: grc

lexer.cpp: lexer.l
	flex -s -o lexer.cpp lexer.l

lexer.o: lexer.cpp lexer.hpp parser.hpp ast.hpp

parser.hpp parser.cpp: parser.y
	bison -dv -o parser.cpp parser.y

parser.o: parser.cpp lexer.hpp ast.hpp

grc: lexer.o parser.o
	$(CC) $(CFLAGS) -o grc lexer.o parser.o

clean:
	$(RM) lexer.cpp parser.cpp parser.hpp parser.output lexer.o parser.o

distclean: clean
	$(RM) grc
