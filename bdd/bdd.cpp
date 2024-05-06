#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <stack>
#include <tuple>

using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::vector;

namespace formula {

class Formula final {
    public:
        typedef enum Kind {
            CONST, // Constant: true or false. For internal use only
            VAR,   // Boolean variable: x[i]
            NOT,   // Negation: ~A
            AND,   // Conjunction: (A1 & A2)
            OR,    // Disjunction: (A1 | A2)
            XOR,   // Exclusive OR: (A1 ^ A2)
            IMPL,  // Implication: (A1 -> A2)
            EQ     // Equivalence: (A1 <-> A2)
        } Kind;
        friend std::ostream& operator <<(std::ostream &out, const Formula &l);

        // Constructor from string. Lexer and parser included
        Formula(std::string ss) {
            // Lexer
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
                if (*i == '(' || *i == ')' || *i == '!' || *i == '&' || *i == '|' || *i == '=' || *i == '^') {
                    tokens.push_back(std::string(i, j+1));
                    i++;
                    j++;
                // Parsing two-symbol operator
                } else if (*i == '-' && *(i+1) == '>') {
                    tokens.push_back(std::string(i, j+2));
                    i += 2;
                    j += 2;
                // Parsing names
                } else if (*i == 'x') {
                    j++;
                    while (j != ss.end() && *j >= '0' && *j <= '9') j++;
                    if (i + 1 == j) {
                        std::cerr << "Error in formula variable :wqname: " << std::endl;
                        std::cerr << ss << std::endl;
                        std::cerr << std::string(i - ss.begin(), ' ') << '^' << std::endl;    
                    }
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

            // Parser
            // Tokens to RPN
            std::stack<Kind> st;
            max_n = 0;

            for (auto s: tokens) {
                if (s == "!") {
                    st.push(Kind::NOT);
                } else if ( s == "|") {
                    while (!st.empty() && (st.top() == Kind::NOT || st.top() == Kind::AND)) {
                        Kind k = st.top();
                        st.pop();
                        nodes.push_back(Node(k));
                    }
                    st.push(Kind::OR);
                } else if ( s == "&") {
                    while (!st.empty() && st.top() == Kind::NOT) {
                        st.pop();
                        nodes.push_back(Node(Kind::NOT));
                    }
                    st.push(Kind::AND);
                } else if (s == "->" || s == "=") {
                    while (!st.empty() && (st.top() == Kind::NOT || st.top() == Kind::AND || st.top() == Kind::OR || st.top() == Kind::XOR)) {
                        Kind k = st.top();
                        st.pop();
                        nodes.push_back(Node(k));
                    }
                    if (s == "=") {
                        st.push(Kind::EQ);
                    } else {
                        st.push(Kind::IMPL);
                    }
                } else if (s == "^") {
                    while (!st.empty() && (st.top() == Kind::NOT || st.top() == Kind::AND || st.top() == Kind::OR)) {
                        Kind k = st.top();
                        st.pop();
                        nodes.push_back(Node(k));
                    }
                    st.push(Kind::XOR);
                } else if ( s == "(") {
                    st.push(Kind::VAR);
                } else if ( s == ")") {
                    while (!st.empty() && (st.top() != Kind::VAR)) {
                        Kind k = st.top();
                        st.pop();
                        nodes.push_back(Node(k));
                    }
                    st.pop();
                } else {
                    size_t no = stoi(std::string(s.begin() + 1, s.end()));
                    max_n = std::max(max_n, no);
                    nodes.push_back(Node(no));
                }
            }
            min_n = 0;
        }

        // (x, T, E, x_no)
        typedef std::tuple<std::string, size_t, size_t, size_t> BDDNode;

        vector<BDDNode>
        BDD() {
            vector<BDDNode> res;

            // 0 and 1
            // Doesn't work properly for constant formulas
            res.emplace_back("0", 0, 0, 0);
            res.emplace_back("1", 1, 1, 0);

            this->apply(res);
            return res;
        }

    private:
        class Node {
            public:
                Kind kind;
                size_t var;
                Node(Kind k) : kind(k), var(0) {};
                Node(bool b) : kind(CONST), var(b ? 1 : 0) {};
                Node(size_t k) : kind(Kind::VAR), var(k) {};
        };
        vector<Node> nodes;
        size_t min_n;        
        size_t max_n;

        // Substitution kind: subformula or substituting constant
        typedef enum SubKind {
            SUBST,
            SUBFORMULA
        } SubKind;

        // Substitution node. Used by simplify formula during substitution
        class SubNode {
            public:
                SubKind kind;
                size_t start, end;
                bool sub;

                SubNode(bool b) : kind(SUBST), start(0), end(0), sub(b) {};
                SubNode(size_t start, size_t end) : kind(SUBFORMULA), start(start), end(end), sub(false) {};
        };

        // Internal constructor of formula for substitute method
        Formula(vector<Node> v, size_t min_n, size_t max_n) : nodes(v), min_n(min_n), max_n(max_n) {}

        // Returns formula with substituted subst(0/1 - false/true) into var with number n
        Formula
        substitute(size_t n, size_t subst) {
            // Main idea: after substitution nodes are splitted in 3 ways:
            // 1) present -- nothing changed, no effect of substituting
            // 2) skipped -- erased by simplifying, e.x. "1 | some_formula" after simplification 
            //               all nodes in subformula "some_formula" will be erased, as well as "|" operation
            // 3) changed -- some operations can be changed, e.x. "subformula = 0" changes into
            //               "!subformula"

            // nodes copy -- needed for changes
            vector<Node> nodes(this->nodes);
            
            // operational stack for simplifier
            std::stack<SubNode> new_nodes;
            
            // skipped node mask
            vector<bool> skipped(nodes.size(), false);

            // Main cycle
            for (size_t i = 0; i < nodes.size(); ++i) {
                if (nodes[i].kind == Formula::VAR && nodes[i].var != n) { // Not substituted var nodes -- present
                    new_nodes.push(SubNode(i, i));
                } else if (nodes[i].kind == NOT) { // NOT -- skipped if const in arg, else present
                    SubNode cur = new_nodes.top();
                    new_nodes.pop();
                    if (cur.kind == SUBST) {
                        new_nodes.push(SubNode(!cur.sub));
                        skipped[i] = true;
                    } else {
                        new_nodes.push(SubNode(cur.start, i));
                    }
                } else if (nodes[i].kind != VAR) { // Other operations...
                    SubNode arg2 = new_nodes.top();
                    new_nodes.pop();
                    SubNode arg1 = new_nodes.top();
                    new_nodes.pop();
                    if (arg1.kind == SUBFORMULA && arg2.kind == SUBFORMULA) { // If no const in args -- present
                        new_nodes.push(SubNode(arg1.start, i));
                    } else if (arg1.kind == SUBST && arg2.kind == SUBST) { // If both are const -- skipped
                        if (nodes[i].kind == AND) new_nodes.push(SubNode(arg1.sub && arg2.sub));
                        if (nodes[i].kind == OR) new_nodes.push(SubNode(arg1.sub || arg2.sub));
                        if (nodes[i].kind == XOR) new_nodes.push(SubNode(arg1.sub ^ arg2.sub));
                        if (nodes[i].kind == EQ) new_nodes.push(SubNode(!(arg1.sub ^ arg2.sub)));
                        if (nodes[i].kind == IMPL) new_nodes.push(SubNode(!arg1.sub || arg2.sub));
                        skipped[i] = true;
                    } else if (nodes[i].kind == IMPL) { // Non-commutative IMPL -- 4 cases...
                        if (arg1.kind == SUBST && arg1.sub == true) {
                            skipped[i] = true;
                            new_nodes.push(arg2);
                        } else if (arg1.kind == SUBST && arg1.sub == false) {
                            for (size_t j = arg2.start; j <= arg2.end; ++j) skipped[j] = true;
                            skipped[i] = true;
                            new_nodes.push(SubNode(true));
                        } else if (arg2.kind == SUBST && arg2.sub == true) {
                            for (size_t j = arg1.start; j <= arg1.end; ++j) skipped[j] = true;
                            skipped[i] = true;
                            new_nodes.push(SubNode(true));
                        } else {
                            arg1.end = i;
                            new_nodes.push(arg1);
                            nodes[i].kind = NOT;
                        } 
                    } else { // Other operations commutative -- make arg1 const. And more cases...
                        if (arg1.kind != SUBST) {
                            std::swap(arg1, arg2);
                        }
                        if (nodes[i].kind == AND) {
                            if (arg1.sub) {
                                skipped[i] = true;
                                new_nodes.push(arg2);                                
                            } else {
                                for (size_t j = arg2.start; j <= arg2.end; ++j) skipped[j] = true;
                                skipped[i] = true;
                                new_nodes.push(SubNode(false));
                            }
                        } else if (nodes[i].kind == OR) {
                            if (arg1.sub) {
                                for (size_t j = arg2.start; j <= arg2.end; ++j) skipped[j] = true;
                                skipped[i] = true;
                                new_nodes.push(SubNode(true));
                            } else {
                                skipped[i] = true;
                                new_nodes.push(arg2);
                            }
                        } else if (nodes[i].kind == XOR) {
                            if (arg1.sub) {
                                nodes[i].kind = NOT;
                                arg2.end = i;
                                new_nodes.push(arg2);
                            } else {
                                skipped[i] = true;
                                new_nodes.push(arg2);
                            }
                        } else if (nodes[i].kind == EQ) {
                            if (!arg1.sub) {
                                nodes[i].kind = NOT;
                                arg2.end = i;
                                new_nodes.push(arg2);
                            } else {
                                skipped[i] = true;
                                new_nodes.push(arg2);
                            }
                        }
                    }
                } else {
                    new_nodes.push(SubNode(subst));
                    skipped[i] = true;
                }
            }

            // Final check if the whole formula is const -- different return value
            SubNode fin = new_nodes.top();
            
            if (fin.kind == SUBST) {
                return Formula(vector<Node>(1, Node(fin.sub)), 0, 0);
            } else {
                vector<Node> res_nodes;
                for (size_t i = 0; i < nodes.size(); ++i) {
                    if (!skipped[i]) res_nodes.push_back(nodes[i]);
                }
                return Formula(res_nodes, n + 1, this->max_n);
            }
        }

        size_t
        apply(vector<BDDNode> &res) {
            // Recursion end
            if (nodes.size() == 1 && nodes[0].kind == CONST) {
                return nodes[0].var;
            }

            // Recursion
            size_t t = this->substitute(this->min_n, 1).apply(res);
            size_t e = this->substitute(this->min_n, 0).apply(res);
            if (t == e) {
                return t;
            } else {
                for (size_t i = 0; i < res.size(); ++i) {
                    if (res[i] == std::make_tuple("x" + std::to_string(this->min_n), t, e, this->min_n)) {
                        return i;
                    }
                }
                res.emplace_back("x" + std::to_string(this->min_n), t, e, this->min_n);
                return res.size() - 1;
            }
        }
};

std::ostream& operator <<(std::ostream &out, const Formula &l) {
    std::stack<std::pair<std::string, int>> res;
    std::pair<std::string, int> arg1, arg2;

    for (Formula::Node el: l.nodes) {
        switch(el.kind) {
            case Formula::Kind::VAR:
                res.emplace("x" + std::to_string(el.var), 0);
                break;
            case Formula::Kind::NOT:
                arg1 = res.top();
                res.pop();
                if (arg1.second > 1) {
                    res.emplace("!(" + arg1.first + ")", 1);
                } else {
                    res.emplace("!" + arg1.first, 1);
                }
                break;
            case Formula::Kind::XOR:
                arg2 = res.top();
                res.pop();
                arg1 = res.top();
                res.pop();
                if (arg1.second > 4) {
                    arg1.first = "(" + arg1.first + ")";
                }
                if (arg2.second > 4) {
                    arg2.first = "(" + arg2.first + ")";
                }
                res.emplace(arg1.first + " ^ " + arg2.first, 4);
                break;
            case Formula::Kind::AND:
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
                res.emplace(arg1.first + " & " + arg2.first, 2);
                break;
            case Formula::Kind::OR:
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
                res.emplace(arg1.first + " | " + arg2.first, 3);
                break;
            case Formula::Kind::IMPL:
                arg2 = res.top();
                res.pop();
                arg1 = res.top();
                res.pop();
                if (arg1.second > 4) {
                    arg1.first = "(" + arg1.first + ")";
                }
                if (arg2.second > 4) {
                    arg2.first = "(" + arg2.first + ")";
                }
                res.emplace(arg1.first + " -> " + arg2.first, 5);
                break;
            case Formula::Kind::EQ:
                arg2 = res.top();
                res.pop();
                arg1 = res.top();
                res.pop();
                if (arg1.second > 4) {
                    arg1.first = "(" + arg1.first + ")";
                }
                if (arg2.second > 4) {
                    arg2.first = "(" + arg2.first + ")";
                }
                res.emplace(arg1.first + " = " + arg2.first, 5);
                break;
            case Formula::Kind::CONST:
                if (el.var == 0) {
                    res.emplace("False", 0);
                } else {
                    res.emplace("True", 0);
                }
                
        }
    }
    out << res.top().first;
    return out;
}

void
BDD_print(vector<Formula::BDDNode> v) {
    cout << "digraph {" << endl;
    cout << "    0 [shape=rect]" << endl;
    cout << "    1 [shape=rect]" << endl;
    for (size_t i = 2; i < v.size(); ++i) {
        std::string name;
        size_t t, e, x_no;
        std::tie(name, t, e, x_no) = v[i];
        cout << "    " << i << " [label=" << name << "] [shape=circle]" << endl;
        cout << "    " << i << "->" << t << endl;
        cout << "    " << i << "->" << e << " [style=dashed]" << endl;
    }

    // Calculate num of levels 
    size_t max_no = 0;
    for (size_t i = 2; i < v.size(); ++i) {
        std::string name;
        size_t t, e, x_no;
        std::tie(name, t, e, x_no) = v[i];
        max_no = std::max(max_no, x_no);
    }

    // Make level
    // TODO: make it faster than O(n^2)
    cout << "    {rank=same; 0 1}" << endl;
    for (size_t cur_no = 0; cur_no <= max_no; ++cur_no) {
        cout << "    {rank=same; ";
        for (size_t i = 2; i < v.size(); ++i) {
            std::string name;
            size_t t, e, x_no;
            std::tie(name, t, e, x_no) = v[i];
            if (x_no == cur_no) {
                cout << " " << i;
            }
        }
        cout << "}" << endl;
    }
    cout << "}" << endl;
}

}

int
main() {
    std::string s;
    std::getline(cin, s);
    auto x = formula::Formula(s).BDD();
    cerr << formula::Formula(s) << endl;
    formula::BDD_print(x);
    return 0;
}


