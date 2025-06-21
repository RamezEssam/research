#include <iostream>
#include <cstdlib>
#include <string>
#include <windows.h>
#include <fstream>
#include <sys/stat.h>
#include "regex.h"

static std::string USAGE = "USAGE: research -p [PATTERN] -i [STRING|INPUT_FILE]";

static int CORRECT_ARG_COUNT = 5;

static inline void print_usage_exit() {
    std::cerr << USAGE << "\n";
    std::exit(1);
}

bool check_file_exists(const char* path) {
    #ifdef _WIN32
    DWORD attrib = GetFileAttributesA(path);
    return (attrib != INVALID_FILE_ATTRIBUTES) &&
        !(attrib & FILE_ATTRIBUTE_DIRECTORY);
    #else
    struct stat path_stat;
    if (stat(path, &path_stat) != 0)
        return false;
    return S_ISREG(path_stat.st_mode);
    #endif
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
    std::string pattern = std::string(argv[2]);
    REGEX::NFA matcher = regex.compile(pattern);

    std::string input = "Brooklyn";




    // If input is file
    if (check_file_exists(argv[4])) {
        std::ifstream file(argv[4]);
        if (!file.is_open()) {
            std::cerr << "Failed to open file.\n";
            return 1;
        }
        std::string line;
        size_t line_n = 1;
        bool matched = false;
        while (std::getline(file, line)) {
            if (regex.match(line, matcher)) {
                std::cout << "Match: " << line_n << ": " << line << "\n";
                matched = true;
            }
            line_n++;
        }
        if(!matched) std::cout << "No Match\n";
        file.close();
    }
    else {
        // otherwise treat input as string literal
        std::string input = std::string(argv[4]);


        if (regex.match(input, matcher)) {
            std::cout << "Match!\n";
        }
        else {
            std::cout << "No match\n";
        }
    }

    

    return 0;
}