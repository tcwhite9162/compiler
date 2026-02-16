#include "lexer.hpp"
#include "parser.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "expected file" << std::endl;
        return -1;
    }

    Arena lexer_arena;
    Arena parser_arena;

    Lexer lexer(argv[1], lexer_arena);
    Parser parser(lexer, parser_arena);

    auto fns = parser.parse();
    parser.print_program(fns);

    return 0;
}
