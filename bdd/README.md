BDD realization for homework 2
-------------------------------

# Usage

Build executable: make
Build and run executable: make run
Clean generates: make clean

# Formula syntax

Program expects single line formula to be passed to stdin.

Formula is a sequence of variables, parenthesis and operators with arbitrary number of space symbols between them.

Variable is a string "x<num>", where <num> is arbitrary positive number in decimal. Consider not to skip any variable numbers for more efficiency.

Parenthesis are symbols "(", ")" for standard operator priority altering.

Operators:
- ! -- unary operator NOT
- | -- binary operator OR
- & -- binary operator AND
- -> -- binary operator IMPLIES
- = -- binary operator EQUIVALENT
- ^ -- binary operator XOR

Operator priority is !, &, |, ^, other. All operators are left associative.
