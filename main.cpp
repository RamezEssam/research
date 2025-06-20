#include <iostream>
#include <cstdlib>
#include <string>
#include "regex.h"

static std::string USAGE = "USAGE: research -p [PATTERN] -i [STRING|INPUT_FILE]";

static int CORRECT_ARG_COUNT = 5;

static inline void print_usage_exit() {
    std::cerr << USAGE << "\n";
    std::exit(1);
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        std::cerr << "No arguments were provided.\n";
        print_usage_exit();
    }
    if (argc != CORRECT_ARG_COUNT) {
        std::cerr << "insufficient/extra number of arguments provided.\n";
        print_usage_exit();
    }

    if (std::string(argv[1]) != "-p") {
        std::cout << "-p" << argv[1] << "\n";
        std::cerr << "incorrect or misplaced flag(s): " << argv[1] << "\n";
        print_usage_exit();
    }

    if (std::string(argv[3]) != "-i") {
        std::cerr << "incorrect or misplaced flag(s): " << argv[3] << "\n";
        print_usage_exit();
    }

    REGEX::Regex regex;

    std::string input = std::string(argv[4]);
    std::string pattern = std::string(argv[2]);

    REGEX::NFA matcher = regex.compile(pattern);

    if (regex.match(input, matcher)) {
        std::cout << "Match!\n";
    }
    else {
        std::cout << "No match\n";
    }

    return 0;
}