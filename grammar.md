# grammar

Program ::= { FunctionDecl }

FunctionDecl ::= "function" Identifier "(" ParamList? ")" "=>" Type Scope

ParamList     ::= Param { "," Param }
Param         ::= Identifier ":" Type

Type ::= Identifier
       | Identifier "<" Type ">"
       | Identifier "<" Type { "," Type } ">"

Scope ::= "{" { Statement } "}"

Statement ::= LetStmt
            | ReturnStmt
            | IfStmt
            | ExprStmt
            | ";"

LetStmt ::= "let" Identifier ":" Type "=" Expr ";"

ReturnStmt ::= "return" Expr? ";"

IfStmt ::= "if" Expr Block

ExprStmt ::= Expr ";"
