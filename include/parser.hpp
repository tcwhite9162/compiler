#pragma once

#include "arena.hpp"
#include "lexer.hpp"

#include <string_view>
#include <vector>

struct ASTNode {
    virtual ~ASTNode() = default;
};

struct Expr : ASTNode {};
struct Stmt : ASTNode {};

struct TypeNode : ASTNode {
    std::string_view name;
    std::vector<TypeNode*> types;
};

struct LiteralExpr : Expr {
    std::string_view value;
};

struct IdentifierExpr : Expr {
    std::string_view name;
};

struct BinaryExpr : Expr {
    Expr* left;
    TokenType op;
    Expr* right;
};

struct UnaryExpr : Expr {
    TokenType op;
    Expr* expr;
};

struct CallExpr : Expr {
    Expr* called;
    std::vector<Expr*> args;
};

struct ParenExpr : Expr {
    Expr* expr;
};

struct LetStmt : Stmt {
    std::string_view name;
    TypeNode* type;
    Expr* expr;
};

struct ReturnStmt : Stmt {
    Expr* value;
};

struct ExprStmt : Stmt {
    Expr* expr;
};

struct ScopeStmt : Stmt {
    std::vector<Stmt*> statements;
};

struct IfStmt : Stmt {
    Expr* condition;
    Stmt* then_branch;
    Stmt* else_branch;
};

struct Param {
    std::string_view name;
    TypeNode* type;
};

struct FunctionDecl : ASTNode {
    std::string_view name;
    std::vector<Param*> params;
    TypeNode* return_type;
    ScopeStmt* body;
};

class Parser {
  public:
    Parser(Lexer& lexer, Arena& arena);

    std::vector<FunctionDecl*> parse();

  private:
    Lexer& lexer;
    Arena& arena;

    Token curr;

    void advance();
    Token expect(TokenType t);

    FunctionDecl* parse_function();
    Param* parse_param();
    TypeNode* parse_type();

    Stmt* parse_stmt();
    ScopeStmt* parse_scope();
    LetStmt* parse_let();
    IfStmt* parse_if();
    ReturnStmt* parse_return();

    ExprStmt* parse_expr_stmt();
    Expr* parse_expr();
};
