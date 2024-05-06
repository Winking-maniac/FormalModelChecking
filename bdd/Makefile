CC=g++
CXX=g++
CXXFLAGS=--std=c++11 -Wall -Werror

all: bdd

bdd: bdd.o

run: bdd
	TMP_DOT=`mktemp`; ./bdd | dot -Tpdf >$$TMP_DOT; xdg-open $$TMP_DOT

.PHONY: clean

clean:
	rm bdd bdd.o
