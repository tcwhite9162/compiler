#pragma once

#include "arena.hpp"
#include "lexer.hpp"

#include <string_view>
#include <vector>

enum class ExprKind {
    Identifier,
    Literal,
    Binary,
    Unary,
    Paren,
    Call,

};

enum class StmtKind {
    Let,
    Return,
    Expr,
    Scope,
    If,
};

struct ASTNode {
    virtual ~ASTNode() = default;
};

struct Expr : ASTNode {
    ExprKind kind;
};
struct Stmt : ASTNode {
    StmtKind kind;
};

struct TypeNode : ASTNode {
    std::string_view name;
    std::vector<TypeNode*> types;
};

struct LiteralExpr : Expr {
    std::string_view value;
    LiteralExpr() { kind = ExprKind::Literal; }
};

struct IdentifierExpr : Expr {
    std::string_view name;
    IdentifierExpr() { kind = ExprKind::Identifier; }
};

struct BinaryExpr : Expr {
    Expr* left;
    TokenType op;
    Expr* right;
    BinaryExpr() { kind = ExprKind::Binary; }
};

struct UnaryExpr : Expr {
    TokenType op;
    Expr* expr;
    UnaryExpr() { kind = ExprKind::Unary; }
};

struct CallExpr : Expr {
    Expr* called;
    std::vector<Expr*> args;
    CallExpr() { kind = ExprKind::Call; }
};

struct ParenExpr : Expr {
    Expr* expr;
    ParenExpr() { kind = ExprKind::Paren; }
};

struct LetStmt : Stmt {
    std::string_view name;
    TypeNode* type;
    Expr* expr;
    LetStmt() { kind = StmtKind::Let; }
};

struct ReturnStmt : Stmt {
    Expr* value;
    ReturnStmt() { kind = StmtKind::Return; }
};

struct ExprStmt : Stmt {
    Expr* expr;
    ExprStmt() { kind = StmtKind::Expr; }
};

struct ScopeStmt : Stmt {
    std::vector<Stmt*> statements;
    ScopeStmt() { kind = StmtKind::Scope; }
};

struct IfStmt : Stmt {
    Expr* condition;
    Stmt* then_branch;
    Stmt* else_branch;
    IfStmt() { kind = StmtKind::If; }
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

enum Precedence : int {
    NONE = 0,
    ASSIGN,
    OR,
    AND,
    EQUALITY,
    COMPARE,
    SUM,
    FACTOR,
    UNARY,
    CALL,
};

class Parser {
  public:
    Parser(Lexer& lexer, Arena& arena);

    std::vector<FunctionDecl*> parse();

    void print_program(const std::vector<FunctionDecl*>& fns);

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

    Expr* parse_prefix();
    Expr* parse_infix(Expr* left, const Token& op, const int prec);
    Expr* parse_call(Expr* from);
    Expr* parse_precedence(const int prec);

    bool is_callable(Expr* expr);
    int get_precedence(TokenType t);

    void indent(const int n);
    void print_expr(Expr* expr, const int indent_level = 0);
    void print_stmt(Stmt* stmt, const int indent_level = 0);
    void print_type(TypeNode* t, const int indent_level = 0);
    void print_function(FunctionDecl* fn);
};
