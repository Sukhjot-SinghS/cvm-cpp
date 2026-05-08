
#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include <iostream>

// 1. Define the Token Types based on your scope
enum class TokenType {
    // Single-character tokens
    PLUS, MINUS, STAR, SLASH, LESS, EQUAL,
    
    // Two-character tokens
    EQUAL_EQUAL,
    
    // Literals
    IDENTIFIER, NUMBER,
    
    // Keywords
    LET, IF, ELSE, WHILE, PRINT, INPUT, TRUE_LITERAL, FALSE_LITERAL,
    LEFT_BRACE, RIGHT_BRACE,
    
    COMMA,          // ,
    LEFT_PAREN,     // (
    RIGHT_PAREN,    // )
    DEF,            // def
    RETURN,     
       // return
    
    // End of File
    EOF_TOKEN
};

// 2. The Token Structure
struct Token {
    TokenType type;
    std::string lexeme;
    int line;

    Token(TokenType type, std::string lexeme, int line) 
        : type(type), lexeme(std::move(lexeme)), line(line) {}

    // Handy for debugging our Lexer output
    void print() const {
        std::cout << "Line " << line << " | Type: " << static_cast<int>(type) 
                  << " | Lexeme: '" << lexeme << "'\n";
    }
};

// 3. The Lexer Class Skeleton
class Lexer {
private:
    std::string source;
    std::vector<Token> tokens;
    
    // State tracking variables
    int start = 0;   // Points to the first character of the current lexeme
    int current = 0; // Points to the character currently being considered
    int line = 1;    // Tracks the current line number for error reporting

    // Helper methods (we'll implement these next)
    bool isAtEnd() const;
    char advance();
    bool match(char expected);
    char peek() const;
    
    void scanToken();
    void addToken(TokenType type);

public:
    Lexer(std::string sourceCode) : source(std::move(sourceCode)) {}

    std::vector<Token> scanTokens() {
        while (!isAtEnd()) {
            // We are at the beginning of the next lexeme.
            start = current;
            scanToken();
        }

        tokens.push_back(Token(TokenType::EOF_TOKEN, "", line));
        return tokens;
    }
};

#endif // LEXER_H
