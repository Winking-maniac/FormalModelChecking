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

#include "ltl.h"
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
using std::vector;
using std::pair;


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
            for (size_t j = nodes[i].arg1.first; j <= nodes[i].arg1.second; ++j) {
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
    for (auto uniq_atom: uniq_name) {
        for(size_t i  = 0; i <= uniq_atom.x_count; ++i) res.emplace_back(uniq_atom.name, i);
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
            while (neg_idx >= 0 && (nodes[neg_idx].kind == NOT || nodes[neg_idx].kind == X)) {
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
            for(size_t j = 0; j < res.size(); ++j) {
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

pair<vector<boost::dynamic_bitset<>>, vector<vector<pair<size_t, bool>>>>
LTL::make_states(std::vector<Atom> &atoms, std::vector<ClosureNode> &closure) {
    if (atoms.size() >= sizeof(unsigned long) * 8) {
        cerr << "Are you sure you want to make a GNBA for not less than a 2^" << atoms.size() << " states?" << endl;
        cerr << "I guess not" << endl;
        exit(1);
    }

    // States in column representation
    vector<boost::dynamic_bitset<>> states(atoms.size() + closure.size());
    // Constraints for transitions in row respresentation
    vector<vector<pair<size_t, bool>>> constraints;

    for(unsigned long i = 0; i < 1ul << atoms.size(); ++i) {
        // Initialize states
        boost::dynamic_bitset<> init_states(atoms.size());
        for (int init_i = atoms.size() - 1; init_i >= 0; init_i--) {
            init_states[init_i] = i & (1 << (atoms.size() - init_i - 1));
        }
        cout << "init_states: " << init_states << endl;
        // Initialize constraints
        vector<pair<size_t, bool>> init_constraints;
        for (size_t i = 0; i < atoms.size(); ++i) {
            if (atoms[i].x_count > 0) {
                init_constraints.emplace_back((size_t)(i - 1), init_states[i]);
            }
        }

        // Initialize stack
        std::stack<pair<boost::dynamic_bitset<>, vector<pair<size_t, bool>>>> q;
        q.emplace(init_states, init_constraints);

        while (!q.empty()) {
            auto cur_states = q.top().first;
            auto cur_constraints = q.top().second;
            q.pop();
            size_t index = cur_states.size() - atoms.size();
            while (index != closure.size()) {
                bool arg1 = (closure[index].neg1 != cur_states[closure[index].arg1]);
                bool arg2 = (closure[index].neg2 != cur_states[closure[index].arg2]);
                if (closure[index].kind == OR) {
                    cur_states.push_back(arg1 || arg2);
                } else if (closure[index].kind == AND) {
                    cur_states.push_back(arg1 && arg2);
                } else if (closure[index].kind == IMPL) {
                    cur_states.push_back(arg1 < arg2);
                } else if (closure[index].kind == F) {
                    if (arg1) {
                        cur_states.push_back(true);
                    } else {
                        // Split
                        break;
                    }
                } else if (closure[index].kind == G) {
                    if (!arg1) {
                        cur_states.push_back(false);
                    } else {
                        // Split
                        break;
                    }
                } else if (closure[index].kind == U) {
                    if (arg2) {
                        cur_states.push_back(true);
                    } else if (!arg1) {
                        cur_states.push_back(false);
                    } else {
                        // Split
                        break;
                    }
                } else if (closure[index].kind == R) {
                    if (!arg2) {
                        cur_states.push_back(false);
                    } else if (arg1) {
                        cur_states.push_back(true);
                    } else {
                        // Split
                        break;
                    }
                }
                index++;
            }
            if (index == closure.size()) {
                cout << "final_state: " << cur_states << endl;
                for (size_t ii = 0; ii < atoms.size() + closure.size(); ++ii) {
                    states[ii].push_back(cur_states[ii]);
                }
                constraints.push_back(cur_constraints);
            } else {
                auto st_false = cur_states;
                auto st_true = cur_states;
                auto cs_false = cur_constraints;
                auto cs_true = cur_constraints;
                st_false.push_back(false);
                st_true.push_back(true);
                cs_false.emplace_back(index + atoms.size(), false);
                cs_true.emplace_back(index + atoms.size(), true);
                q.emplace(st_true, cs_true);
                q.emplace(st_false, cs_false);
            }
        }
    }
    return std::make_pair(states, constraints);
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
    auto states_constraints = this->make_states(atoms, closure);
    auto states = states_constraints.first;
    auto constraints = states_constraints.second;
    // Initial states
    auto initial = states[states.size() - 1];
    bool whole_true = true;
    for (size_t i = nodes.size() - 1; nodes[i].kind == NOT || nodes[i].kind == X; --i) {
        if (nodes[i].kind == NOT) {
            whole_true = !whole_true;
        }
    }

    for (const auto& x: states) {
        cout << x << endl;
    }
    for (const auto& x: constraints) {
        for (auto y: x) {
            cout << y.first << ": " << y.second << ", ";
        }
        cout << endl;
    }

    // Finally, building automaton
    fsm::Automaton buchi;
    // Setting states and initial states
    for (size_t i = 0; i < initial.size(); ++i) {
        std::string state = "s" + std::to_string(i);
        buchi.add_state(state);
        if (initial[i] == whole_true) {
            buchi.set_initial(state);
        }
    }
    // Setting transitions
    for (size_t i = 0; i < initial.size(); ++i) {
        // Get source name
        std::string source = "s" + std::to_string(i);
        // Get label set
        std::set<std::string> label;
        for (size_t j = 0; j < atoms.size(); ++j) {
            if (atoms[j].x_count == 0 && states[j][i]) {
                label.insert(atoms[j].name);
            }
        }
        // Initialize transitions
        boost::dynamic_bitset<> trans(initial.size());
        trans.set();
        // Get transitions as conjunction of constraints
        for (const auto &j : constraints[i]) {
            trans &= (j.second ? states[j.first] : ~states[j.first]);
        }
        for (size_t j = 0; j < trans.size(); ++j) {
            if (trans[j]) {
                buchi.add_trans(source, label, "s" + std::to_string(j));
            }
        }
    }
    // Setting final states
    for (size_t i = atoms.size(); i < states.size(); ++i) {
        auto kind = closure[i - atoms.size()].kind;
        if (kind == F || kind == G || kind == U || kind == R) {
            for (size_t j = 0; j < initial.size(); ++j) {
                bool is_split = false;
                for (const auto &constraint: constraints[j]) {
                    if (constraint.first == i) is_split = true;
                }
                std::string state = "s" + std::to_string(j);
                if (kind == F || kind == U) {
                    if (!states[i][j] || !is_split) buchi.set_final(state, i);
                } else {
                    if (states[i][j] || !is_split) buchi.set_final(state, i);
                }
            }
        }
    }



    return buchi;
}

} // namespace model::ltl
