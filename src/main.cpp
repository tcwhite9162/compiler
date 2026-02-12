#include "lexer.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "expected file" << std::endl;
        return -1;
    }

    Arena arena;

    Lexer lexer(argv[1], arena);
    Token curr = lexer.next();

    while (curr.is_not(TokenType::FileEnd)) {
        curr.print();
        curr = lexer.next();
    }

    return 0;
}
