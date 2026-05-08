
#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include <memory>
#include <vector>
#include <iostream>

// --- 1. Abstract Syntax Tree (AST) Nodes ---

// Base class for all Expressions
struct Expr {
    virtual ~Expr() = default;
    virtual void printAST() const = 0; // For debugging our tree
};

// A simple number or boolean literal (e.g., 10, true)
struct LiteralExpr : public Expr {
    Token value;
    LiteralExpr(Token val) : value(val) {}
    
    void printAST() const override { std::cout << value.lexeme; }
};
struct InputExpr : public Expr {
    InputExpr() = default;
    void printAST() const override { std::cout << "(input)\n"; }
};

// A variable name (e.g., x)
struct VariableExpr : public Expr {
    Token name;
    VariableExpr(Token n) : name(n) {}
    
    void printAST() const override { std::cout << name.lexeme; }
};

// A binary operation (e.g., left + right)
struct BinaryExpr : public Expr {
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;

    BinaryExpr(std::unique_ptr<Expr> l, Token o, std::unique_ptr<Expr> r)
        : left(std::move(l)), op(o), right(std::move(r)) {}

    void printAST() const override {
        std::cout << "(" << op.lexeme << " ";
        left->printAST();
        std::cout << " ";
        right->printAST();
        std::cout << ")";
    }
};

// Base class for all Statements
struct Stmt {
    virtual ~Stmt() = default;
    virtual void printAST() const = 0;
};

// A print statement: print <expr>
struct PrintStmt : public Stmt {
    std::unique_ptr<Expr> expression;
    PrintStmt(std::unique_ptr<Expr> expr) : expression(std::move(expr)) {}

    void printAST() const override {
        std::cout << "(print ";
        expression->printAST();
        std::cout << ")\n";
    }
};

struct BlockStmt : public Stmt {
    std::vector<std::unique_ptr<Stmt>> statements;
    BlockStmt(std::vector<std::unique_ptr<Stmt>> stmts) : statements(std::move(stmts)) {}
    void printAST() const override { std::cout << "{ ... block ... }\n"; }
};

struct IfStmt : public Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> thenBranch;
    std::unique_ptr<Stmt> elseBranch; // Can be null

    IfStmt(std::unique_ptr<Expr> c, std::unique_ptr<Stmt> t, std::unique_ptr<Stmt> e)
        : condition(std::move(c)), thenBranch(std::move(t)), elseBranch(std::move(e)) {}
    void printAST() const override { std::cout << "(if ...)\n"; }
};

struct WhileStmt : public Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> body;

    WhileStmt(std::unique_ptr<Expr> c, std::unique_ptr<Stmt> b)
        : condition(std::move(c)), body(std::move(b)) {}
    void printAST() const override { std::cout << "(while ...)\n"; }
};

// A variable declaration: let <name> = <expr>
struct VarStmt : public Stmt {
    Token name;
    std::unique_ptr<Expr> initializer; // Can be null if just 'let x;'

    VarStmt(Token n, std::unique_ptr<Expr> init) : name(n), initializer(std::move(init)) {}

    void printAST() const override {
        std::cout << "(let " << name.lexeme << " = ";
        if (initializer) initializer->printAST();
        std::cout << ")\n";
    }
};

// A Function Definition: def add(a, b) { ... }
struct FunctionStmt : public Stmt {
    Token name;
    std::vector<Token> params; // The arguments: a, b
    std::vector<std::unique_ptr<Stmt>> body; // The block of code inside

    FunctionStmt(Token n, std::vector<Token> p, std::vector<std::unique_ptr<Stmt>> b)
        : name(n), params(std::move(p)), body(std::move(b)) {}
    void printAST() const override { std::cout << "(def " << name.lexeme << " ...)\n"; }
};

// A Return Statement: return x + 1
struct ReturnStmt : public Stmt {
    std::unique_ptr<Expr> value; // What are we returning?
    ReturnStmt(std::unique_ptr<Expr> val) : value(std::move(val)) {}
    void printAST() const override { std::cout << "(return ...)\n"; }
};

// A Function Call Expression: add(10, 5)
struct CallExpr : public Expr {
    Token callee; // The name of the function being called
    std::vector<std::unique_ptr<Expr>> arguments; // The values passed in

    CallExpr(Token c, std::vector<std::unique_ptr<Expr>> args)
        : callee(c), arguments(std::move(args)) {}
    void printAST() const override { std::cout << "(call " << callee.lexeme << " ...)\n"; }
};

// --- 2. The Parser Engine ---

class Parser {
private:
    std::vector<Token> tokens;
    int current = 0; // Points to the current token we are looking at

    // Helper methods (very similar to the Lexer's pointers!)
    Token peek() const;
    Token previous() const;
    bool isAtEnd() const;
    bool check(TokenType type) const;
    Token advance();
    bool match(std::vector<TokenType> types);
    Token consume(TokenType type, const std::string& message);

    // Recursive Descent parsing methods
    std::unique_ptr<Stmt> declaration();
    std::unique_ptr<Stmt> varDeclaration();
    std::unique_ptr<Stmt> statement();
    std::unique_ptr<Stmt> printStatement();
    
    std::unique_ptr<Expr> expression();
    std::unique_ptr<Expr> equality();
    std::unique_ptr<Expr> comparison();
    std::unique_ptr<Expr> term();
    std::unique_ptr<Expr> factor();
    std::unique_ptr<Expr> primary();

public:
    Parser(const std::vector<Token>& tokens) : tokens(tokens) {}

    // Kicks off the parsing process, returning a list of syntax trees (Statements)
    std::vector<std::unique_ptr<Stmt>> parse() {
        std::vector<std::unique_ptr<Stmt>> statements;
        while (!isAtEnd()) {
            statements.push_back(declaration());
        }
        return statements;
    }
};

#endif // PARSER_H