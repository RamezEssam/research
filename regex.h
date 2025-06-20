// regex.h
#ifndef REGEX_H
#define REGEX_H

#include <string>
#include <set>

namespace REGEX {
    struct State {
        char transition;
        State* out1 = nullptr;
        State* out2 = nullptr;
    };

    struct NFA {
        State* start;
        State* accept;
    };

    class Regex {
    public:
        Regex();
        NFA compile(std::string src);
        bool match(std::string& input, NFA nfa);

    private:
        State* start;
        State* accept;
        State* new_state(char transition);
        int precedence(char op);
        bool isLiteral(char c);
        std::string insertConcat(std::string regex);
        std::string preprocessEscapes(const std::string& regex);
        std::string toPostfix(std::string regex);
        NFA buildNFA(std::string postfix);
        void addEpsilon(State* s, std::set<State*>& states);
        bool simulate(NFA nfa, const std::string& input);


    };
}



#endif // REGEX_H
