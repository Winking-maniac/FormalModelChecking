CC=gcc
CXX=gcc
CXXFLAGS=-Wall -Werror -Ofast

all: dpll

dpll: dpll.c
	gcc ${CXXFLAGS} -fprofile-generate -o dpll-tmp dpll.c
	./dpll-tmp <hanoi4.cnf >/dev/null
	gcc ${CXXFLAGS} -fprofile-use -o dpll dpll.c

run: dpll
	/usr/bin/time -v ./dpll <hanoi4.cnf

.PHONY: clean

clean:
	rm dpll dpll-tmp dpll.gcda
