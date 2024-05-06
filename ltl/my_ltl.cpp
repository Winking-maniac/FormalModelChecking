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

#include "my_ltl.h"
#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <queue>
#include <cassert>
#include <boost/dynamic_bitset.hpp>

using std::cout;
using std::cerr;
using std::endl;

namespace model::ltl {

LTL::LTL(std::string &ss) {
    std::vector<std::string> tokens;
    
    auto i = ss.begin();
    auto j = ss.begin();
  
    tokens.push_back("("); 
    while (i != ss.end()) {
        // Skipping spaces
        while (*i == ' ') {
            i++;
            j++;
        }
        // Parsing one-symbol operators
        if (*i == '(' || *i == ')' || *i == 'F' || *i == 'G' || *i == 'U' || *i == 'X' || *i == 'R' || *i == '!') {
            tokens.push_back(std::string(i, j+1));
            i++;
            j++;
        // Parsing two-symbol operators
        } else if ((*i == '&' && *(i+1) == '&') || (*i == '|' && *(i+1) == '|') || (*i == '-' && *(i+1) == '>')) {
            tokens.push_back(std::string(i, j+2));
            i += 2;
            j += 2;
        // Parsing names
        } else if (*i >= 'a' && *i <= 'z') {
            while (j != ss.end() && *j >= 'a' && *j <= 'z') j++;
            tokens.push_back(std::string(i, j));
            i = j;
        // Error
        } else {
            std::cerr << "Error in formula: " << std::endl;
            std::cerr << ss << std::endl;
            std::cerr << std::string(i - ss.begin(), ' ') << '^' << std::endl;
        }   
    }    
    tokens.push_back(")"); 
    
    // Tokens to RPN
    std::stack<LTL::Kind> st;

    for (auto s: tokens) std::cout << "Token: " << s << std::endl;

    for (auto s: tokens) {
        //std::cout << "Token: " << s << std::endl;

        //if (!st.empty()) std::cout << "st top: " << st.top() << std::endl;
        if (s == "!") {
            st.push(LTL::Kind::NOT);
        } else if (s == "X") {
            st.push(LTL::Kind::X);
        } else if ( s == "G") {
            st.push(LTL::Kind::G);
        } else if ( s == "F") {
            st.push(LTL::Kind::F);
        } else if ( s == "||") {
            while (!st.empty() && (st.top() == LTL::Kind::NOT || st.top() == LTL::Kind::AND)) {
                LTL::Kind k = st.top();
                st.pop();
                if (k == LTL::Kind::NOT) {
                    nodes.push_back(Node(LTL::Kind::NOT));
                    nodes[nodes.size() - 1].arg1 = nodes[nodes.size() - 2].ind;
                    nodes[nodes.size() - 1].ind = nodes[nodes.size() - 2].ind;
                    nodes[nodes.size() - 1].ind.second++; 
                } else {
                    nodes.push_back(Node(LTL::Kind::AND));
                    nodes[nodes.size() - 1].arg2 = nodes[nodes.size() - 2].ind;
                    nodes[nodes.size() - 1].arg1 = nodes[nodes[nodes.size() - 2].ind.first - 1].ind;
                    nodes[nodes.size() - 1].ind = std::pair<unsigned, unsigned>(nodes[nodes.size() - 1].arg1.first, nodes[nodes.size() - 1].arg2.second);
                }
            }
            st.push(LTL::Kind::OR);
        } else if ( s == "&&") {
            while (!st.empty() && st.top() == LTL::Kind::NOT) {
                st.pop();
                nodes.push_back(Node(LTL::Kind::NOT));
                nodes[nodes.size() - 1].arg1 = nodes[nodes.size() - 2].ind;
                nodes[nodes.size() - 1].ind = nodes[nodes.size() - 2].ind;
                nodes[nodes.size() - 1].ind.second++; 
            }
            st.push(LTL::Kind::AND);
        } else if ( s == "->" || s == "U" || s == "R") {
            while (!st.empty() && (st.top() == LTL::Kind::NOT || st.top() == LTL::Kind::AND || st.top() == LTL::Kind::OR)) {
                LTL::Kind k = st.top();
                st.pop();
                if (k == LTL::Kind::NOT) {
                    nodes.push_back(Node(LTL::Kind::NOT));
                    nodes[nodes.size() - 1].arg1 = nodes[nodes.size() - 2].ind;
                    nodes[nodes.size() - 1].ind = nodes[nodes.size() - 2].ind;
                    nodes[nodes.size() - 1].ind.second++; 
                } else { 
                    nodes.push_back(Node(k));
                    nodes[nodes.size() - 1].arg2 = nodes[nodes.size() - 2].ind;
                    nodes[nodes.size() - 1].arg1 = nodes[nodes[nodes.size() - 2].ind.first - 1].ind;
                    nodes[nodes.size() - 1].ind = std::pair<unsigned, unsigned>(nodes[nodes.size() - 1].arg1.first, nodes[nodes.size() - 1].arg2.second);
                }
            }
            if (s == "U") {
                st.push(LTL::Kind::U);
            } else if (s == "R") {
                st.push(LTL::Kind::R);
            } else if (s == "->") {
                st.push(LTL::Kind::IMPL);
            }
        } else if ( s == "(") {
            st.push(LTL::Kind::ATOM);
        } else if ( s == ")") {
            while (!st.empty() && (st.top() != LTL::Kind::ATOM)) {
                LTL::Kind k = st.top();
                st.pop();
                if (k == LTL::Kind::NOT || k == LTL::Kind::F || k == LTL::Kind::G || k == LTL::Kind::X) {
                    nodes.push_back(Node(k));
                    nodes[nodes.size() - 1].arg1 = nodes[nodes.size() - 2].ind;
                    nodes[nodes.size() - 1].ind = nodes[nodes.size() - 2].ind;
                    nodes[nodes.size() - 1].ind.second++; 
                } else {
                    nodes.push_back(Node(k));
                    nodes[nodes.size() - 1].arg2 = nodes[nodes.size() - 2].ind;
                    nodes[nodes.size() - 1].arg1 = nodes[nodes[nodes.size() - 2].ind.first - 1].ind;
                    nodes[nodes.size() - 1].ind = std::pair<unsigned, unsigned>(nodes[nodes.size() - 1].arg1.first, nodes[nodes.size() - 1].arg2.second + 1);
                    assert(nodes[nodes.size() - 1].arg2.second + 1 == nodes.size() - 1);
                }
            }
            st.pop();
        } else {
            nodes.push_back(Node(LTL::Kind::ATOM));
            nodes[nodes.size() - 1].ind = std::pair<unsigned, unsigned>(nodes.size() - 1, nodes.size() - 1);
            nodes[nodes.size() - 1].name = s;
        }    
    }
}

std::ostream& operator <<(std::ostream &out, const LTL &l) {
    std::stack<std::pair<std::string, int>> res;
    std::pair<std::string, int> arg1, arg2;

    for(LTL::Node el: l.nodes) std::cout << el.kind << " ";
    std::cout << std::endl;

    for (LTL::Node el: l.nodes) {
        switch(el.kind) {
            case LTL::Kind::ATOM:
                res.emplace(el.name, 0);
                break;
            case LTL::Kind::NOT:
                arg1 = res.top();
                res.pop();
                if (arg1.second > 1) {
                    res.emplace("!(" + arg1.first + ")", 1);
                } else {
                    res.emplace("!" + arg1.first, 1);
                }
                break;
            case LTL::Kind::F:
                arg1 = res.top();
                res.pop();
                res.emplace("F(" + arg1.first + ")", 4);
                break;
            case LTL::Kind::G:
                arg1 = res.top();
                res.pop();
                res.emplace("G(" + arg1.first + ")", 4);
                break;
            case LTL::Kind::X:
                arg1 = res.top();
                res.pop();
                res.emplace("X(" + arg1.first + ")", 4);
                break;
            case LTL::Kind::AND:
                arg2 = res.top();
                res.pop();
                arg1 = res.top();
                res.pop();
                if (arg1.second > 2) {
                    arg1.first = "(" + arg1.first + ")";
                }
                if (arg2.second > 2) {
                    arg2.first = "(" + arg2.first + ")";
                }
                res.emplace(arg1.first + " && " + arg2.first, 2);
                break;
            case LTL::Kind::OR:
                arg2 = res.top();
                res.pop();
                arg1 = res.top();
                res.pop();
                if (arg1.second > 3) {
                    arg1.first = "(" + arg1.first + ")";
                }
                if (arg2.second > 3) {
                    arg2.first = "(" + arg2.first + ")";
                }
                res.emplace(arg1.first + " || " + arg2.first, 3);
                break;
            case LTL::Kind::IMPL:
                arg2 = res.top();
                res.pop();
                arg1 = res.top();
                res.pop();
                if (arg1.second > 3) {
                    arg1.first = "(" + arg1.first + ")";
                }
                if (arg2.second > 3) {
                    arg2.first = "(" + arg2.first + ")";
                }
                res.emplace(arg1.first + " -> " + arg2.first, 4);
                break;
            case LTL::Kind::U:
                arg2 = res.top();
                res.pop();
                arg1 = res.top();
                res.pop();
                if (arg1.second > 3) {
                    arg1.first = "(" + arg1.first + ")";
                }
                if (arg2.second > 3) {
                    arg2.first = "(" + arg2.first + ")";
                }
                res.emplace(arg1.first + " U " + arg2.first, 4);
                break;
            case LTL::Kind::R:
                arg2 = res.top();
                res.pop();
                arg1 = res.top();
                res.pop();
                if (arg1.second > 3) {
                    arg1.first = "(" + arg1.first + ")";
                }
                if (arg2.second > 3) {
                    arg2.first = "(" + arg2.first + ")";
                }
                res.emplace(arg1.first + " R " + arg2.first, 4);
                break;
        }
    }
    out << res.top().first;
    return out;
} 

void
LTL::propagate_x() {
    for (size_t i = 0; i < this->nodes.size(); ++i) {
        if (nodes[i].kind == X) {
            for (int j = nodes[i].arg1.first; j <= nodes[i].arg1.second; ++j) {
                if (nodes[j].kind == ATOM) nodes[j].x_count++;
            }
        }
    }
}

std::vector<LTL::Atom>
LTL::make_atoms() {
    std::vector<Atom> uniq_name;
    for (auto node: nodes) {
        if (node.kind == ATOM) {
            bool found = false;
            for (auto &atom: uniq_name) {
                if (atom.name == node.name) {
                    atom.x_count = std::max(atom.x_count, node.x_count);
                    found = true;
                    break;
                }
            }
            if (!found) {
                uniq_name.emplace_back(node.name, node.x_count);
            }
        }
    }
    std::vector<Atom> res;
    for (auto uatom: uniq_name) {
        for(size_t i  = 0; i <= uatom.x_count; ++i) res.emplace_back(uatom.name, i);
    }
    return res;
}

std::vector<LTL::ClosureNode>
LTL::make_closure(std::vector<Atom> &atoms) {
    std::vector<LTL::ClosureNode> res;
    std::vector<size_t> node2closure(nodes.size());
    for (size_t i = 0; i < nodes.size(); ++i) {
        if (nodes[i].kind == ATOM) {
            for(size_t j = 0; j < atoms.size(); ++j) {
                if (nodes[i].x_count == atoms[j].x_count && nodes[i].name == atoms[j].name) {
                    node2closure[i] = j;
                    break;
                }
            }
        } else if (nodes[i].kind == X) {
            node2closure[i] = node2closure[i-1];
        } else if (nodes[i].kind == NOT) {
            node2closure[i] = node2closure[i-1];
        } else {
            size_t arg1 = node2closure[nodes[i].arg1.second];
            size_t arg2;
            if (nodes[i].kind == G || nodes[i].kind == F) {
                arg2 = arg1;
            } else {
                arg2 = node2closure[nodes[i].arg2.second];
            }
            
            bool neg1 = false, neg2 = false;
            int neg_idx = nodes[i].arg1.second;
            while (neg_idx >= 0 && nodes[neg_idx].kind == NOT || nodes[neg_idx].kind == X) {
                if (nodes[neg_idx].kind == NOT) neg1 = !neg1;
                neg_idx--;
            }

            neg_idx = nodes[i].arg2.second;
            while (neg_idx >= 0 && nodes[neg_idx].kind == NOT) {
                neg2 = !neg2;
                neg_idx--;
            }

            if (nodes[i].kind == AND || nodes[i].kind == OR) {
                if (arg1 > arg2) {
                    std::swap(arg1, arg2);
                    std::swap(neg1, neg2);
                }
            }

            bool found = false;
            for(int j = 0; j < res.size(); ++j) {
                if (res[j] == ClosureNode(nodes[i].kind, arg1, arg2, neg1, neg2)) {
                    node2closure[i] = j + atoms.size();
                    found = true;
                    break;
                }
            }
            if (!found) {
                res.emplace_back(nodes[i].kind, arg1, arg2, neg1, neg2);
                node2closure[i] = atoms.size() + res.size() - 1;
            }
        }
    }
    return res;
}

std::vector<boost::dynamic_bitset<>>
LTL::make_states(std::vector<Atom> &atoms, std::vector<ClosureNode> &closure) {
    std::vector<boost::dynamic_bitset<>> res;
    if (atoms.size() >= sizeof(unsigned long) * 8) {
        cerr << "Are you sure you want to make a GNBA for not less than a 2^" << atoms.size() << " states?" << endl;
        cerr << "I guess not" << endl;
        exit(1);
    }

    for(unsigned long i = 0; i < 1ul << atoms.size(); ++i) {
        boost::dynamic_bitset<> cur(atoms.size(), i);
        std::queue<boost::dynamic_bitset<>> q;
        q.
    }

}

fsm::Automaton
LTL::make_buchi() {
    this->propagate_x();
    
    auto atoms = this->make_atoms();
    cout << "Atoms:" << endl;
    for (auto atom: atoms) cout << atom.name << " " << atom.x_count << endl;
    auto closure = this->make_closure(atoms);
    cout << "Closure:" << endl;
    for (auto a: closure) cout << a.kind << " " << a.arg1 << " " << a.arg2 << " " << a.neg1 << " " << a.neg2 << endl;
    auto states = this->make_states();
    
    
    return fsm::Automaton();
}

} // namespace model::ltl
