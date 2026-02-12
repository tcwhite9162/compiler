#include "parser.hpp"
#include "arena.hpp"
#include "lexer.hpp"

// #include <string_view>
#include <vector>

Parser::Parser(Lexer& lexer, Arena& arena) : lexer(lexer), arena(arena), curr(Token(TokenType::Unknown, "", -1)) {
    advance();
}

void Parser::advance() {
    curr = lexer.next();
    while (curr.type == TokenType::Comment)
        curr = lexer.next();
}

Token Parser::expect(TokenType t) {
    if (curr.type != t) {
        std::cerr << "Parser error on line " << curr.line << std::endl;
        std::cerr << "Expected: " << type_to_string(t) << std::endl;
        std::cerr << "Got: " << type_to_string(curr.type) << std::endl;
        std::exit(-1);
    }
    Token out = curr;
    advance();
    return out;
}

std::vector<FunctionDecl*> Parser::parse() {
    std::vector<FunctionDecl*> functions;

    while (curr.type != TokenType::FileEnd) {
        functions.push_back(parse_function());
    }

    return functions;
}

FunctionDecl* Parser::parse_function() {
    FunctionDecl* fn = arena.alloc<FunctionDecl>();

    expect(TokenType::Function);
    fn->name = expect(TokenType::Identifier).value;

    expect(TokenType::LeftParen);
    if (curr.type != TokenType::RightParen) {
        fn->params.push_back(parse_param());
        while (curr.type == TokenType::Comma) {
            advance();
            fn->params.push_back(parse_param());
        }
    }
    expect(TokenType::RightParen);
    expect(TokenType::Arrow);

    fn->return_type = parse_type();
    fn->body        = parse_scope();

    return fn;
}

Param* Parser::parse_param() {
    Param* param = arena.alloc<Param>();

    param->name = expect(TokenType::Identifier).value;
    expect(TokenType::Colon);
    param->type = parse_type();

    return param;
}

TypeNode* Parser::parse_type() {
    TypeNode* type = arena.alloc<TypeNode>();

    type->name = expect(TokenType::Identifier).value;
    if (curr.type == TokenType::LessThan) {
        advance();
        type->types.push_back(parse_type());

        while (curr.type == TokenType::Comma) {
            advance();
            type->types.push_back(parse_type());
        }
        expect(TokenType::GreaterThan);
    }
    return type;
}

ScopeStmt* Parser::parse_scope() {
    ScopeStmt* stmt = arena.alloc<ScopeStmt>();

    expect(TokenType::LeftCurly);
    while (curr.type != TokenType::RightCurly) {
        stmt->statements.push_back(parse_stmt());
    }
    expect(TokenType::RightCurly);

    return stmt;
}

LetStmt* Parser::parse_let() {
    LetStmt* stmt = arena.alloc<LetStmt>();

    expect(TokenType::Let);
    stmt->name = expect(TokenType::Identifier).value;
    expect(TokenType::Colon);
    stmt->type = parse_type();
    expect(TokenType::Equal);
    stmt->expr = parse_expr();
    expect(TokenType::SemiColon);

    return stmt;
}

IfStmt* Parser::parse_if() {
    IfStmt* stmt      = arena.alloc<IfStmt>();
    stmt->else_branch = nullptr;

    expect(TokenType::If);

    stmt->condition   = parse_expr();
    stmt->then_branch = parse_scope();

    if (curr.type == TokenType::Else) {
        advance();

        if (curr.type == TokenType::If) {
            stmt->else_branch = parse_if();
        }
        else {
            stmt->else_branch = parse_scope();
        }
    }

    return stmt;
}

ReturnStmt* Parser::parse_return() {
    ReturnStmt* stmt = arena.alloc<ReturnStmt>();

    expect(TokenType::Return);
    stmt->value = parse_expr();
    expect(TokenType::SemiColon);

    return stmt;
}

ExprStmt* Parser::parse_expr_stmt() {
    ExprStmt* expr = arena.alloc<ExprStmt>();

    expr->expr = parse_expr();
    expect(TokenType::SemiColon);

    return expr;
}

Expr* Parser::parse_expr() {
    Expr* expr = arena.alloc<Expr>();

    // TODO: Pratt parser for expressions

    return expr;
}

Stmt* Parser::parse_stmt() {
    switch (curr.type) {
    case TokenType::Let:
        return parse_let();
    case TokenType::Return:
        return parse_return();
    case TokenType::If:
        return parse_if();

    default:
        return parse_expr_stmt();
    }
}
