/*
 * Copyright 2024 Winking-maniac (http://github.com/Winking-maniac)
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except
 * in compliance with the License. You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License
 * is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
 * or implied. See the License for the specific language governing permissions and limitations under
 * the License.
 */

#pragma once

#include <iostream>
#include <vector>
#include <utility>
#include "fsm.h"

namespace model::ltl {


class LTL final {
public:
    LTL(std::string &s);
    LTL() = delete;

    fsm::Automaton make_buchi();

    friend std::ostream& operator <<(std::ostream &out, const LTL &l);
private:
    enum Kind {
        ATOM, // Atomic proposition: p
        NOT,  // Negation: ~A
        AND,  // Conjunction: (A1 & A2)
        OR,   // Disjunction: (A1 | A2)
        IMPL, // Implication: (A1 -> A2)
        X,    // NeXt time: X{A}
        G,    // Globally: G{A}
        F,    // In the Future: F{A}
        U,    // Until: (A1 U A2)
        R     // Release: (A1 R A2)
    };

    struct Node { 
        
        Node(Kind k) : kind(k), x_count(0) {std::cout << "Node " << k << " created" << std::endl;};
        Kind kind;
        std::string name;
        std::pair<unsigned, unsigned> ind;  // indexes of whole subformula
        std::pair<unsigned, unsigned> arg1; // indexes of subformula for left(or the only) argument of operators
        std::pair<unsigned, unsigned> arg2; // indexes of subformula for right argument of &, |, ->, U, R
        size_t x_count;
    };

    struct Atom {
        std::string name;
        size_t x_count;
        Atom(std::string s, size_t count) : name(s), x_count(count) {}
    };

    struct ClosureNode {
        Kind kind;
        size_t arg1; // index in atoms + closure 
        size_t arg2; // index in atoms + closure
        bool neg1, neg2;

        ClosureNode(Kind k, size_t a1, size_t a2, bool n1, bool n2) : kind(k), arg1(a1),
                arg2(a2), neg1(n1), neg2(n2) {}

        bool operator==(ClosureNode n) {
            return n.kind == this->kind && n.arg1 == this->arg1 && n.arg2 == this->arg2 &&
            n.neg1 == this->neg1 && n.neg2 == this->neg2;
        }
    };
    
    std::vector<Node> nodes = std::vector<Node>();

    void propagate_x();
    std::vector<Atom> make_atoms();
    std::vector<ClosureNode> make_closure(std::vector<Atom> &);
    std::vector<boost::dynamic_bitset<>> make_states(std::vector<Atom> &, std::vector<ClosureNode> &);
};


} // namespace model::ltl
