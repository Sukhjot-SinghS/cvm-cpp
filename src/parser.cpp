#include "parser.h"
#include <stdexcept>

// --- Helper Methods (The Parser's Pointers) ---

Token Parser::peek() const {
    return tokens[current];
}

Token Parser::previous() const {
    return tokens[current - 1];
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::EOF_TOKEN;
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

Token Parser::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

bool Parser::match(std::vector<TokenType> types) {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    Token badToken = peek();
    throw std::runtime_error("\n[Syntax Error] " + message + " (Found '" + badToken.lexeme + "')"); 
}

// --- Statement Parsing ---

std::unique_ptr<Stmt> Parser::declaration() {
    if (match({TokenType::DEF})) {
        Token name = consume(TokenType::IDENTIFIER, "Expect function name.");
        consume(TokenType::LEFT_PAREN, "Expect '(' after function name.");
        
        std::vector<Token> parameters;
        if (!check(TokenType::RIGHT_PAREN)) {
            do {
                parameters.push_back(consume(TokenType::IDENTIFIER, "Expect parameter name."));
            } while (match({TokenType::COMMA}));
        }
        consume(TokenType::RIGHT_PAREN, "Expect ')' after parameters.");
        consume(TokenType::LEFT_BRACE, "Expect '{' before function body.");
        
        std::vector<std::unique_ptr<Stmt>> body;
        while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
            body.push_back(declaration());
        }
        consume(TokenType::RIGHT_BRACE, "Expect '}' after function body.");
        
        return std::make_unique<FunctionStmt>(name, std::move(parameters), std::move(body));
    }
    if (match({TokenType::LET})) return varDeclaration();
    return statement();
}

std::unique_ptr<Stmt> Parser::varDeclaration() {
    Token name = consume(TokenType::IDENTIFIER, "Expect variable name.");
    
    std::unique_ptr<Expr> initializer = nullptr;
    if (match({TokenType::EQUAL})) {
        initializer = expression();
    }

    return std::make_unique<VarStmt>(name, std::move(initializer));
}

std::unique_ptr<Stmt> Parser::statement() {
    if (match({TokenType::IF})) {
        std::unique_ptr<Expr> condition = expression();
        std::unique_ptr<Stmt> thenBranch = statement();
        std::unique_ptr<Stmt> elseBranch = nullptr;
        if (match({TokenType::ELSE})) {
            elseBranch = statement();
        }
        return std::make_unique<IfStmt>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
    }
    
    if (match({TokenType::WHILE})) {
        std::unique_ptr<Expr> condition = expression();
        std::unique_ptr<Stmt> body = statement();
        return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
    }

    if (match({TokenType::LEFT_BRACE})) {
        std::vector<std::unique_ptr<Stmt>> statements;
        while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
            statements.push_back(declaration());
        }
        consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");
        return std::make_unique<BlockStmt>(std::move(statements));
    }

    if (match({TokenType::PRINT})) return printStatement();
    if (match({TokenType::RETURN})) {
        std::unique_ptr<Expr> value = nullptr;
        if (!check(TokenType::RIGHT_BRACE)) { // If it's not just an empty return
            value = expression();
        }
        return std::make_unique<ReturnStmt>(std::move(value));
    }
    
    throw std::runtime_error("Only let, print, if, while, and blocks are supported right now!");
}

std::unique_ptr<Stmt> Parser::printStatement() {
    std::unique_ptr<Expr> value = expression();
    return std::make_unique<PrintStmt>(std::move(value));
}

// --- Expression Parsing (Recursive Descent) ---

std::unique_ptr<Expr> Parser::expression() {
    return equality();
}

std::unique_ptr<Expr> Parser::equality() {
    std::unique_ptr<Expr> expr = comparison();

    while (match({TokenType::EQUAL_EQUAL})) {
        Token op = previous();
        std::unique_ptr<Expr> right = comparison();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::comparison() {
    std::unique_ptr<Expr> expr = term();

    while (match({TokenType::LESS})) {
        Token op = previous();
        std::unique_ptr<Expr> right = term();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::term() {
    std::unique_ptr<Expr> expr = factor();

    // Handles + and -
    while (match({TokenType::PLUS, TokenType::MINUS})) {
        Token op = previous();
        std::unique_ptr<Expr> right = factor();
        // The old expr becomes the left child of the new Binary node!
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::factor() {
    std::unique_ptr<Expr> expr = primary();

    // Handles * and / (Multiplication happens BEFORE addition because it's deeper in the call stack)
    while (match({TokenType::STAR, TokenType::SLASH})) {
        Token op = previous();
        std::unique_ptr<Expr> right = primary();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::primary() {
    if (match({TokenType::FALSE_LITERAL, TokenType::TRUE_LITERAL, TokenType::NUMBER})) {
        return std::make_unique<LiteralExpr>(previous());
    }
    if (match({TokenType::INPUT})) {
        return std::make_unique<InputExpr>();
    }
    if (match({TokenType::IDENTIFIER})) {
        Token name = previous();
        
        // Is it a function call?
        if (match({TokenType::LEFT_PAREN})) {
            std::vector<std::unique_ptr<Expr>> arguments;
            if (!check(TokenType::RIGHT_PAREN)) {
                do {
                    arguments.push_back(expression());
                } while (match({TokenType::COMMA}));
            }
            consume(TokenType::RIGHT_PAREN, "Expect ')' after arguments.");
            return std::make_unique<CallExpr>(name, std::move(arguments));
        }
        
        // Or just a normal variable?
        return std::make_unique<VariableExpr>(name);
    }


    throw std::runtime_error("Expect expression at line " + std::to_string(peek().line));
}