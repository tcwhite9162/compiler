#include "lexer.hpp"

#include <cctype>
#include <fstream>

static std::optional<TokenType> keyword_lookup(std::string_view s) {
    if (s == "function")
        return TokenType::Function;
    if (s == "let")
        return TokenType::Let;
    if (s == "if")
        return TokenType::If;
    if (s == "else")
        return TokenType::Else;
    if (s == "return")
        return TokenType::Return;

    return std::nullopt;
}

Token Lexer::get_identifier() noexcept {
    const char* start = position;

    while (is_ident_char(peek())) {
        advance();
    }

    std::string_view txt = arena.copy(start, position - start);

    if (auto keyword = keyword_lookup(txt))
        return Token(keyword.value(), txt, curr_line);

    return Token(TokenType::Identifier, txt, curr_line);
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

Token Lexer::equal_or_arrow() noexcept {
    const char* start = position;

    advance();
    if (peek() == '>') {
        advance();
        return Token(TokenType::Arrow, arena.copy(start, 2), curr_line);
    }

    return Token(TokenType::Equal, arena.copy(start, 1), curr_line);
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

    if (std::isalpha(static_cast<unsigned char>(peek())) || peek() == '_')
        return get_identifier();

    if (is_digit(peek()))
        return get_number();

    switch (peek()) {
    case '\0':
        return Token(TokenType::FileEnd, "", curr_line);
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
    case '=':
        return equal_or_arrow();

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
