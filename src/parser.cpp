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

void Parser::print_program(const std::vector<FunctionDecl*>& fns) {
    for (auto* fn : fns) {
        print_function(fn);
        std::cout << std::endl;
    }
}

FunctionDecl* Parser::parse_function() {

    expect(TokenType::Function);
    std::string_view name = expect(TokenType::Identifier).value;
    expect(TokenType::LeftParen);

    FunctionDecl* fn = arena.alloc<FunctionDecl>();
    fn->name         = name;

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
        if (curr.type == TokenType::FileEnd) {
            std::cerr << "error: expected `}` on line " << curr.line << std::endl;
            std::exit(-1);
        }
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
    return parse_precedence(Precedence::ASSIGN);
}

Expr* Parser::parse_precedence(const int min_prec) {
    Expr* left = parse_prefix();

    while (true) {
        int prec = get_precedence(curr.type);
        if (prec < min_prec)
            break;

        Token op = curr;
        advance();

        int next_prec = prec + 1;

        left = parse_infix(left, op, next_prec);
    }

    return left;
}

Expr* Parser::parse_prefix() {
    switch (curr.type) {
    case TokenType::Identifier: {
        std::string_view name = expect(TokenType::Identifier).value;
        IdentifierExpr* id    = arena.alloc<IdentifierExpr>();
        id->name              = name;
        return id;
    }

    case TokenType::Number: {
        LiteralExpr* literal = arena.alloc<LiteralExpr>();
        literal->value       = expect(TokenType::Number).value;
        return literal;
    }

    case TokenType::LeftParen: {
        advance();
        ParenExpr* p = arena.alloc<ParenExpr>();
        p->expr      = parse_expr();
        expect(TokenType::RightParen);
        return p;
    }

    case TokenType::Exclamation: {
        Token op         = expect(TokenType::Exclamation);
        Expr* right      = parse_precedence(Precedence::UNARY);
        UnaryExpr* unary = arena.alloc<UnaryExpr>();
        unary->op        = op.type;
        unary->expr      = right;
        return unary;
    }

    case TokenType::Minus: {
        Token op         = expect(TokenType::Minus);
        Expr* right      = parse_precedence(Precedence::UNARY);
        UnaryExpr* unary = arena.alloc<UnaryExpr>();
        unary->op        = op.type;
        unary->expr      = right;
        return unary;
    }

    default:
        std::cerr << "Unexpected token on line " << curr.line << ": " << type_to_string(curr.type) << std::endl;
        std::exit(-1);
    }
}

Expr* Parser::parse_infix(Expr* left, const Token& op, const int prec) {
    if (op.type == TokenType::LeftParen) {
        if (!is_callable(left)) {
            std::cerr << "cannot call non callable expression" << std::endl;
            std::exit(-1);
        }
        return parse_call(left);
    }

    Expr* right = parse_precedence(prec);

    BinaryExpr* bin = arena.alloc<BinaryExpr>();
    bin->left       = left;
    bin->op         = op.type;
    bin->right      = right;

    return bin;
}

Expr* Parser::parse_call(Expr* from) {
    CallExpr* call = arena.alloc<CallExpr>();
    call->called   = from;

    if (curr.type != TokenType::RightParen) {
        call->args.push_back(parse_expr());
        while (curr.type == TokenType::Comma) {
            advance();
            call->args.push_back(parse_expr());
        }
    }

    expect(TokenType::RightParen);
    return call;
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

bool Parser::is_callable(Expr* expr) {
    switch (expr->kind) {

    case ExprKind::Identifier:
        return true;

    case ExprKind::Paren:
        return is_callable(static_cast<ParenExpr*>(expr)->expr);

    default:
        return false;
    }
}

int Parser::get_precedence(TokenType t) {
    switch (t) {

    case TokenType::LeftParen:
        return Precedence::CALL;

    case TokenType::Exclamation:
        return Precedence::UNARY;

    case TokenType::LessThan:
        [[fallthrough]];
    case TokenType::GreaterThan:
        return Precedence::COMPARE;

    case TokenType::Plus:
        [[fallthrough]];
    case TokenType::Minus:
        return Precedence::SUM;

    case TokenType::Asterisk:
        [[fallthrough]];
    case TokenType::Slash:
        return Precedence::FACTOR;

    default:
        return Precedence::NONE;
    }
}

void Parser::indent(const int n) {
    for (int i = 0; i < n; i++)
        std::cout << "  ";
}
void Parser::print_expr(Expr* expr, const int indent_level) {
    indent(indent_level);

    switch (expr->kind) {

    case ExprKind::Identifier: {
        auto id = static_cast<IdentifierExpr*>(expr);
        std::cout << "Identifier (" << id->name << ")" << std::endl;
        break;
    }

    case ExprKind::Literal: {
        auto lit = static_cast<LiteralExpr*>(expr);
        std::cout << "Literal (" << lit->value << ")" << std::endl;
        break;
    }

    case ExprKind::Unary: {
        auto un = static_cast<UnaryExpr*>(expr);
        std::cout << "Unary (" << type_to_string(un->op) << ")" << std::endl;
        print_expr(un->expr, indent_level + 1);
        break;
    }

    case ExprKind::Binary: {
        auto bin = static_cast<BinaryExpr*>(expr);
        std::cout << "Binary (" << type_to_string(bin->op) << ")" << std::endl;

        indent(indent_level);
        std::cout << "left:" << std::endl;
        print_expr(bin->left, indent_level + 1);

        indent(indent_level);
        std::cout << "right:" << std::endl;
        print_expr(bin->right, indent_level + 1);

        break;
    }

    case ExprKind::Call: {
        auto call = static_cast<CallExpr*>(expr);
        std::cout << "Call " << std::endl;

        indent(indent_level + 1);
        std::cout << "callee:" << std::endl;
        print_expr(call->called, indent_level + 2);

        indent(indent_level + 1);
        std::cout << "args:" << std::endl;
        for (auto* arg : call->args) {
            print_expr(arg, indent_level + 2);
        }
        break;
    }

    case ExprKind::Paren: {
        auto paren = static_cast<ParenExpr*>(expr);
        std::cout << "Paren" << std::endl;

        print_expr(paren->expr, indent_level + 1);
        break;
    }
    }
}

void Parser::print_stmt(Stmt* s, int indent_level) {
    indent(indent_level);

    switch (s->kind) {

    case StmtKind::Let: {
        auto* let = static_cast<LetStmt*>(s);
        std::cout << "Let " << let->name << " : " << let->type->name << std::endl;
        print_expr(let->expr, indent_level + 1);
        break;
    }

    case StmtKind::Return: {
        auto* ret = static_cast<ReturnStmt*>(s);
        std::cout << "Return\n";
        print_expr(ret->value, indent_level + 1);
        break;
    }

    case StmtKind::Expr: {
        auto* es = static_cast<ExprStmt*>(s);
        std::cout << "ExprStmt\n";
        print_expr(es->expr, indent_level + 1);
        break;
    }

    case StmtKind::Scope: {
        auto* sc = static_cast<ScopeStmt*>(s);
        std::cout << "Scope\n";
        for (auto* st : sc->statements)
            print_stmt(st, indent_level + 1);
        break;
    }

    case StmtKind::If: {
        auto* iff = static_cast<IfStmt*>(s);
        std::cout << "If\n";

        indent(indent_level + 1);
        std::cout << "condition:\n";
        print_expr(iff->condition, indent_level + 2);

        indent(indent_level + 1);
        std::cout << "then:\n";
        print_stmt(iff->then_branch, indent_level + 2);

        if (iff->else_branch) {
            indent(indent_level + 1);
            std::cout << "else:\n";
            print_stmt(iff->else_branch, indent_level + 2);
        }
        break;
    }
    }
}

void Parser::print_type(TypeNode* t, const int indent_level) {
    indent(indent_level);
    std::cout << t->name;

    if (!t->types.empty()) {
        std::cout << "<";
        for (size_t i = 0; i < t->types.size(); i++) {
            print_type(t->types[i]);
            if (i + 1 < t->types.size()) {
                std::cout << ", ";
            }
        }
        std::cout << ">";
    }
}

void Parser::print_function(FunctionDecl* fn) {
    std::cout << "Function " << fn->name << std::endl;

    std::cout << "  params:\n";
    for (auto* p : fn->params) {
        std::cout << "    " << p->name << " : ";
        print_type(p->type);
        std::cout << std::endl;
    }

    std::cout << "  return: ";
    print_type(fn->return_type, 1);
    std::cout << std::endl;

    std::cout << "  body:\n";
    print_stmt(fn->body, 2);
}
