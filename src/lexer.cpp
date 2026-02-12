#include "../include/lexer.hpp"

#include <fstream>

Token Lexer::get_identifier() noexcept {
    const char* start = position;

    while (is_ident_char(peek())) {
        advance();
    }

    return Token(TokenType::Identifier, arena.copy(start, position - start), curr_line);
}

Token Lexer::get_number() noexcept {
    const char* start = position;
    while (is_digit(peek())) {
        advance();
    }

    return Token(TokenType::Number, arena.copy(start, position - start), curr_line);
}

Token Lexer::atom(TokenType t) noexcept {
    const char* start = position;
    advance();

    return Token(t, arena.copy(start, 1), curr_line);
}

Token Lexer::comment() noexcept {
    const char* start = position;
    advance();

    while (peek() != '\0' && peek() != '\n')
        advance();

    return Token(TokenType::Comment, arena.copy(start, position - start), curr_line);
}

Token Lexer::next() noexcept {
    while (is_whitespace(peek())) {
        if (peek() == '\n')
            curr_line++;
        advance();
    }

    if (is_ident_char(peek()) || peek() == '_')
        return get_identifier();

    if (is_digit(peek()))
        return get_number();

    switch (peek()) {
    case '\0':
        return Token(TokenType::FileEnd, arena.copy(position, 1), curr_line);
    case '(':
        return atom(TokenType::LeftParen);
    case ')':
        return atom(TokenType::RightParen);
    case '{':
        return atom(TokenType::LeftCurly);
    case '}':
        return atom(TokenType::RightCurly);
    case '[':
        return atom(TokenType::LeftSquare);
    case ']':
        return atom(TokenType::RightSquare);
    case '<':
        return atom(TokenType::LessThan);
    case '>':
        return atom(TokenType::GreaterThan);
    case '=':
        return atom(TokenType::Equal);
    case '+':
        return atom(TokenType::Plus);
    case '-':
        return atom(TokenType::Minus);
    case '*':
        return atom(TokenType::Asterisk);
    case '/':
        return atom(TokenType::Slash);
    case '.':
        return atom(TokenType::Dot);
    case ',':
        return atom(TokenType::Comma);
    case '"':
        return atom(TokenType::DoubleQuote);
    case '\'':
        return atom(TokenType::SingleQuote);
    case ';':
        return atom(TokenType::SemiColon);
    case ':':
        return atom(TokenType::Colon);
    case '#':
        return comment();

    default:
        return atom(TokenType::Unknown);
    }
}

char* Lexer::open_file(const char* path) {
    std::streamsize size;
    std::ifstream file(path, std::ios::binary | std::ios::ate);

    if (!file.is_open()) {
        std::cerr << "error opening file " << path << std::endl;
        return nullptr;
    }

    size = file.tellg();
    file.seekg(0, std::ios::beg);

    char* buffer = new char[size + 1];

    if (!file.read(buffer, size)) {
        std::cerr << "error: could not read entire file" << std::endl;
        delete[] buffer;
        return nullptr;
    }

    buffer[size] = '\0';
    file.close();
    return buffer;
}
