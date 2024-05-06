/*
 * Copyright 2021 ISP RAS (http://www.ispras.ru)
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
#include <string>
#include <sstream>
#include <stack>

namespace model::ltl {

std::vector<std::unique_ptr<const Formula>> Formula::_formulae;

std::ostream& operator <<(std::ostream &out, const Formula &formula) {
  switch (formula.kind()) {
  case Formula::ATOM:
    return out << formula.prop();
  case Formula::NOT:
    return out << "!(" << formula.arg() << ")";
  case Formula::AND:
    return out << "(" << formula.lhs() << ") && (" << formula.rhs() << ")";
  case Formula::OR:
    return out << "(" << formula.lhs() << ") || (" << formula.rhs() << ")";
  case Formula::IMPL:
    return out << "(" << formula.lhs() << ") -> (" << formula.rhs() << ")";
  case Formula::X:
    return out << "X(" << formula.arg() << ")";
  case Formula::G:
    return out << "G(" << formula.arg() << ")";
  case Formula::F:
    return out << "F(" << formula.arg() << ")";
  case Formula::U:
    return out << "(" << formula.lhs() << ") U (" << formula.rhs() << ")";
  case Formula::R:
    return out << "(" << formula.lhs() << ") R (" << formula.rhs() << ")";
  }

  return out;
}

const Formula get_formula() {
    std::string s;
    std::getline(std::cin, s);
    
    std::stringstream ss;
    ss << "( " + s + " )";

    std::stack<Formula> f;
    std::stack<Formula::Kind> st;

    while(ss >> s) {
        std::cout << "Token:  "<< s << std::endl;

        if (!f.empty()) std::cout << f.top() << std::endl;
        if (!st.empty()) std::cout << "st top: " << st.top() << std::endl;
        if (s == "!") {
            st.push(Formula::NOT);
        } else if ( s == "||") {
            while (!st.empty() && (st.top() == Formula::NOT || st.top() == Formula::AND)) {
                Formula::Kind k = st.top();
                st.pop();
                if (k == Formula::NOT) {
                    Formula arg1 = std::move(f.top());
                    f.pop();
                    f.push(!(arg1));
                } else {
                    Formula arg1 = std::move(f.top());
                    f.pop();
                    Formula arg2 = std::move(f.top());
                    f.pop();
                    f.push(arg1 && arg2);
                }
            }
            st.push(Formula::OR);
        } else if ( s == "&&") {
            st.pop();
            while (!st.empty() && st.top() == Formula::NOT) {
                Formula arg1 = std::move(f.top());
                f.pop();
                f.push(!(arg1));
            }
            st.push(Formula::AND);
        } else if ( s == "->" || s == "X" || s == "G" || s == "F" || s == "U" || s == "R") {
            while (!st.empty() && (st.top() == Formula::NOT || st.top() == Formula::AND || st.top() == Formula::OR)) {
                Formula::Kind k = st.top();
                st.pop();
                if (k == Formula::NOT) {
                    Formula arg1 = std::move(f.top());
                    f.pop();
                    f.push(!(arg1));
                } else if (k == Formula::AND) {
                    Formula arg1 = std::move(f.top());
                    f.pop();
                    Formula arg2 = std::move(f.top());
                    f.pop();
                    f.push(arg1 && arg2);
                } else { 
                    Formula arg1 = std::move(f.top());
                    f.pop();
                    Formula arg2 = std::move(f.top());
                    f.pop();
                    f.push(arg1 || arg2);
                }
            }
            if (s == "X") {
                st.push(Formula::X);
            } else if ( s == "G") {
                st.push(Formula::G);
            } else if ( s == "F") {
                st.push(Formula::F);
            } else if ( s == "U") {
                st.push(Formula::U);
            } else if ( s == "R") {
                st.push(Formula::R);
            } else if ( s == "->") {
                st.push(Formula::IMPL);
            }
        } else if ( s == "(") {
            st.push(Formula::ATOM);
        } else if ( s == ")") {
            while (!st.empty() && (st.top() != Formula::ATOM)) {
                Formula::Kind k = st.top();
                st.pop();
                if (k == Formula::NOT) {
                    Formula arg1 = std::move(f.top());
                    f.pop();
                    f.push(!(arg1));
                } else if (k == Formula::AND) {
                    Formula arg1 = std::move(f.top());
                    f.pop();
                    Formula arg2 = std::move(f.top());
                    std::cout << arg1 << arg2;
                    f.pop();
                    f.push(arg2 && arg1);
                } else if (k == Formula::OR) {
                    Formula arg1 = std::move(f.top());
                    f.pop();
                    Formula arg2 = std::move(f.top());
                    f.pop();
                    f.push(arg2 || arg1);
                } else if (k == Formula::IMPL) {
                    Formula arg1 = f.top();
                    f.pop();
                    Formula arg2 = f.top();
                    f.pop();
                    f.push(arg2 >> arg1);
                } else if (k == Formula::F) {
                    //std::cout << "in F" << std::endl;
                    Formula arg1 = f.top();
                    //std::cout << arg1 << std::endl;
                    f.pop();
                    f.push(F(arg1));
                } else if (k == Formula::G) {
                    Formula arg1 = std::move(f.top());
                    f.pop();
                    f.push(G(arg1));
                } else if (k == Formula::X) {
                    Formula arg1 = std::move(f.top());
                    f.pop();
                    f.push(X(arg1));
                } else if (k == Formula::U) {
                    Formula arg1 = std::move(f.top());
                    f.pop();
                    Formula arg2 = std::move(f.top());
                    f.pop();
                    f.push(U(arg2, arg1));
                } else if (k == Formula::R) {
                    Formula arg1 = std::move(f.top());
                    f.pop();
                    Formula arg2 = std::move(f.top());
                    f.pop();
                    f.push(R(arg2, arg1));
                }
            }
            st.pop();
        } else {
            f.push(P(s));
        }
    }
    return f.top();
}

} // namespace model::ltl
