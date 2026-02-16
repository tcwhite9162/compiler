#pragma once

#include "arena.hpp"
#include <iostream>

enum class TokenType : int {
    Number = 0,
    Identifier,

    Plus,
    Minus,
    Asterisk,
    Slash,
    Equal,
    LessThan,
    GreaterThan,
    Exclamation,

    Dot,
    Comma,
    Colon,
    SemiColon,
    SingleQuote,
    DoubleQuote,

    LeftParen,
    RightParen,
    LeftCurly,
    RightCurly,
    LeftSquare,
    RightSquare,

    Function,
    Arrow,
    Let,
    If,
    Else,
    Return,

    Comment,
    FileEnd,

    Unknown
};

inline const char* type_to_string(TokenType t) {
    static const char* names[] = {"Number",     "Identifier",  "Plus",        "Minus",     "Asterisk",   "Slash",
                                  "Equal",      "LessThan",    "GreaterThan", "Exclamation",  "Dot",       "Comma",      "Colon",
                                  "SemiColon",  "SingleQuote", "DoubleQuote", "LeftParen", "RightParen", "LeftCurly",
                                  "RightCurly", "LeftSquare",  "RightSquare", "Function",  "Arrow",      "Let",
                                  "If",         "Else",        "Return",      "Comment",   "FileEnd",    "Unknown"};

    return names[static_cast<int>(t)];
}

struct Token {
  public:
    explicit Token(TokenType _type, std::string_view val, const int line) noexcept : type(_type), value(val), line(line) {}

    [[nodiscard]] bool is(TokenType t) const noexcept { return type == t; }
    [[nodiscard]] bool is_not(TokenType t) const noexcept { return type != t; }

    void set_value(std::string_view val) noexcept { value = std::move(val); }

    void print() const noexcept { std::cout << line << "| " << type_to_string(type) << ": " << value << std::endl; }

    TokenType type;
    std::string_view value;
    int line;
};

class Lexer {
  public:
    Lexer(const char* file_path, Arena& arena) : arena(arena) {
        position = open_file(file_path);
        start    = position;

        if (!position) {
            std::cerr << "error reading file " << file_path << std::endl;
            std::exit(-1);
        }
    }

    ~Lexer() { delete[] start; }

    Token next() noexcept;
    [[nodiscard]] int get_line() const noexcept { return curr_line; }

  private:
    Arena& arena;

    const char* start    = nullptr;
    const char* position = nullptr;
    int curr_line        = 1;

    char* open_file(const char* path);

    Token get_identifier() noexcept;
    Token get_number() noexcept;
    Token atom(TokenType t) noexcept;
    Token comment() noexcept;
    Token equal_or_arrow() noexcept;

    char peek() const { return *position; };
    char advance() { return *position++; }
};

inline bool is_whitespace(const char c) {
    return std::isspace(static_cast<unsigned char>(c));
}

inline bool is_digit(const char c) {
    return std::isdigit(static_cast<unsigned char>(c));
}

inline bool is_ident_char(const char c) {
    return (std::isalnum(static_cast<unsigned char>(c)) || c == '_');
}
