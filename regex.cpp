// regex.cpp
#include "regex.h"
#include <stack>
#include <queue>
#include <iostream>

const char CONCAT_OP = '\x01';           // used for implicit concat in postfix
const char LITERAL_DOT = '\x02';         // marks escaped '.'
const char LITERAL_BACKSLASH = '\x03';   // marks escaped '\'


namespace REGEX {
    // Defualt constructor
    Regex::Regex() : start(nullptr), accept(nullptr) {
        // Constructor body is empty — initializes state pointers
    }

    // Public methods
    NFA Regex::compile(std::string src) {
        std::string preprocessed = preprocessEscapes(src);
        std::string postfix = toPostfix(preprocessed);

        NFA nfa = buildNFA(postfix);

        return nfa;

    }

    bool Regex::match(std::string& input, NFA nfa) {
        return simulate(nfa, input);
    }

    //// Private methods

    State* Regex::new_state(char transition) {
        State* s = new State();
        s->transition = transition;
        return s;
    }

    int Regex::precedence(char op) {
        if (op == '*' || op == '+') return 3;
        if (op == CONCAT_OP) return 2; // explicit concatenation
        if (op == '|') return 1;
        return 0;
    }

    bool Regex::isLiteral(char c) {
        return !(c == '(' || c == ')' || c == '*' || c == '+' || c == '|' || c == CONCAT_OP || c == '\\');
    }

    std::string Regex::preprocessEscapes(const std::string& regex) {
        std::string output;
        bool escape = false;
        for (char c : regex) {
            if (escape) {
                if (c == '.') {
                    output += LITERAL_DOT;
                }
                else if (c == '\\') {
                    output += LITERAL_BACKSLASH;
                }
                else {
                    output += c; // future: add support for \d, \w, etc.
                }
                escape = false;
            }
            else if (c == '\\') {
                escape = true;
            }
            else {
                output += c;
            }
        }
        if (escape) output += LITERAL_BACKSLASH; // handle trailing '\'
        return output;
    }



    std::string Regex::insertConcat(std::string regex) {
        std::string output;
        for (size_t i = 0; i < regex.size(); ++i) {
            output += regex[i];
            if (i + 1 < regex.size()) {
                char curr = regex[i];
                char next = regex[i + 1];
                if ((isLiteral(curr) || curr == '*' || curr == '+' || curr == ')') &&
                    (isLiteral(next) || next == '(')) {
                    output += CONCAT_OP;
                }
            }
        }
        return output;
    }

    std::string Regex::toPostfix(std::string regex) {

        regex = insertConcat(regex);
        std::stack<char> ops;
        std::string output;

        for (char c : regex) {
            if (isLiteral(c)) {
                output += c;
            }
            else if (c == '(') {
                ops.push(c);
            }
            else if (c == ')') {
                while (!ops.empty() && ops.top() != '(') {
                    output += ops.top();
                    ops.pop();
                }
                ops.pop(); // pop '('
            }
            else if (c == CONCAT_OP || c == '*' || c == '+' || c == '|') {
                while (!ops.empty() && precedence(ops.top()) >= precedence(c)) {
                    output += ops.top();
                    ops.pop();
                }
                ops.push(c);
            }
            else {
                while (!ops.empty() && precedence(ops.top()) >= precedence(c)) {
                    output += ops.top();
                    ops.pop();
                }
                ops.push(c);
            }
        }
        while (!ops.empty()) {
            output += ops.top();
            ops.pop();
        }
        return output;
    }

    NFA Regex::buildNFA(std::string postfix) {
        std::stack<NFA> st;

        for (int i = 0; i < postfix.size(); ++i) {
            char c = postfix[i];
            if (isLiteral(c)) {
                State* s1 = new_state(0);
                State* s2 = new_state(0);
                s1->transition = c;
                s1->out1 = s2;
                NFA nfa = { s1, s2 };
                st.push(nfa);
            }
            else if (c == CONCAT_OP) {
                if (st.size() < 2) { std::cerr << "Invalid regex\n"; std::exit(1); }
                NFA nfa2 = st.top(); st.pop();
                NFA nfa1 = st.top(); st.pop();
                nfa1.accept->transition = 0;
                nfa1.accept->out1 = nfa2.start;
                st.push({ nfa1.start, nfa2.accept });
            }
            else if (c == '|') {
                if (st.size() < 2) { std::cerr << "Invalid regex\n"; std::exit(1); }
                NFA nfa2 = st.top(); st.pop();
                NFA nfa1 = st.top(); st.pop();
                State* s = new_state(0);
                State* accept = new_state(0);
                s->out1 = nfa1.start;
                s->out2 = nfa2.start;
                nfa1.accept->transition = 0;
                nfa1.accept->out1 = accept;
                nfa2.accept->transition = 0;
                nfa2.accept->out1 = accept;
                st.push({ s, accept });
            }
            else if (c == '+') {
                NFA nfa1 = st.top(); st.pop();

                State* s = new_state(0);       // new start state
                State* accept = new_state(0);  // new accept state

                s->out1 = nfa1.start;          // must enter nfa1 once
                nfa1.accept->transition = 0;
                nfa1.accept->out1 = nfa1.start;  // loop back to start
                nfa1.accept->out2 = accept;      // or go to accept

                st.push({ s, accept });
            }
            else if (c == '*') {
                NFA nfa1 = st.top();
                st.pop();
                State* s = new_state(0);
                State* accept = new_state(0);
                s->out1 = nfa1.start;
                s->out2 = accept;
                nfa1.accept->transition = 0;
                nfa1.accept->out1 = nfa1.start;
                nfa1.accept->out2 = accept;
                st.push({ s, accept });
            }
        }
        return st.top();
    }

    void Regex::addEpsilon(State* s, std::set<State*>& states) {
        std::queue<State*> q;
        q.push(s);
        while (!q.empty()) {
            State* curr = q.front(); q.pop();
            if (states.count(curr)) continue;
            states.insert(curr);
            if (curr->transition == 0) {
                if (curr->out1) q.push(curr->out1);
                if (curr->out2) q.push(curr->out2);
            }
        }
    }

    bool Regex::simulate(NFA nfa, const std::string& input) {
        std::set<State*> currStates;
        addEpsilon(nfa.start, currStates);

        for (char c : input) {
            std::set<State*> nextStates;
            for (State* state : currStates) {
                if (state->out1) {
                    if (state->transition == c) {
                        // exact literal match
                        addEpsilon(state->out1, nextStates);
                    }
                    else if (state->transition == '.' /* wildcard */) {
                        // wildcard match (any character)
                        addEpsilon(state->out1, nextStates);
                    }
                    else if (state->transition == LITERAL_DOT && c == '.') {
                        // escaped literal dot
                        addEpsilon(state->out1, nextStates);
                    }
                    else if (state->transition == LITERAL_BACKSLASH && c == '\\') {
                        // escaped literal backslash
                        addEpsilon(state->out1, nextStates);
                    }
                }
            }
            currStates = nextStates;
        }

        return currStates.count(nfa.accept) > 0;
    }


}