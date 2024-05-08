LTL to Buchi automaton realization for homework 1
-------------------------------
# Description

Solution is made with fully custom LTL library. FSM library has no changes.

LTL library provides class of LTL formula with 3 possible methods of interaction:
- Constructor from string representation
- Output operator
- Method make_buchi(), returning Buchi automaton for formula

Internal representation of formula is reverse polish notation of Nodes.
No formula simplification -- the only transformation is to X-less and NOT-less version during constructing closure.
WARNING: it may cause state misorder with canonical solutions(ones with only X and U in closure). 
Another state misorder can be due to atoms order.

E.g, atoms set { p, q, Xp, Xq, XXq } will have order p, Xp, q, Xq, XXq.

So the result automaton is equivalent to one received via canonical algorithm up to a permutation of states.

# Prerequisites

Boost library is used, so it should be installed. 
For Ubuntu 20.04, 
```
sudo apt install libboost-dev
```

# Usage

Build executable: make

Build and run executable: make run

Clean generates: make clean

# Formula syntax

Formula is a sequence of variables, parenthesis and operators with arbitrary number of space symbols between them.

Variable is a string of lowercase letters with arbitrary length.

Parenthesis are symbols "(", ")" for standard operator priority altering.

Operators:
- ! -- unary operator NOT
- || -- binary operator OR
- && -- binary operator AND
- -> -- binary operator IMPLIES
Temporary operators:
- F -- unary operator FUTURE
- G -- unary operator GLOBALLY
- U -- binary operator UNTIL
- R -- binary operator RELEASE

Operator priority is !, &&, ||, { ->, U, R }, {F, G}. All operators are left associative.
