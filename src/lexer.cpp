#include "lexer.h"
#include <cctype>
#include <unordered_map>

// Map to hold our custom language's reserved keywords
const std::unordered_map<std::string, TokenType> keywords = {
    {"let", TokenType::LET},
    {"if", TokenType::IF},
    {"else", TokenType::ELSE},
    {"while", TokenType::WHILE},
    {"print", TokenType::PRINT},
    {"input", TokenType::INPUT},
    {"true", TokenType::TRUE_LITERAL},
    {"false", TokenType::FALSE_LITERAL},
    {"def", TokenType::DEF},
    {"return", TokenType::RETURN}
};

// --- Helper Methods ---

bool Lexer::isAtEnd() const {
    return static_cast<size_t>(current) >= source.length();
}

char Lexer::advance() {
    return source[current++];
}

char Lexer::peek() const {
    if (isAtEnd()) return '\0';
    return source[current];
}

bool Lexer::match(char expected) {
    if (isAtEnd()) return false;
    if (source[current] != expected) return false;
    current++;
    return true;
}

void Lexer::addToken(TokenType type) {
    std::string text = source.substr(start, current - start);
    tokens.push_back(Token(type, text, line));
}

// --- Main Scanning Logic ---

void Lexer::scanToken() {
    char c = advance();
    switch (c) {
        // Single-character tokens
        case '+': addToken(TokenType::PLUS); break;
        case '-': addToken(TokenType::MINUS); break;
        case '*': addToken(TokenType::STAR); break;
        case ',': addToken(TokenType::COMMA); break;
        case '(': addToken(TokenType::LEFT_PAREN); break;
        case ')': addToken(TokenType::RIGHT_PAREN); break;
        case '/': addToken(TokenType::SLASH); break;
        case '{': addToken(TokenType::LEFT_BRACE); break;
        case '}': addToken(TokenType::RIGHT_BRACE); break;
        // One or two character tokens
        case '=':
            addToken(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);
            break;
        case '<':
            addToken(TokenType::LESS);
            break;

        // Ignore whitespace
        case ' ':
        case '\r':
        case '\t':
            break;
        case '\n':
            line++;
            break;

        default:
            if (std::isdigit(c)) {
                // Consume all consecutive digits for a NUMBER
                while (std::isdigit(peek())) advance();
                addToken(TokenType::NUMBER);
            } 
            else if (std::isalpha(c)) {
                // Consume alphanumeric chars for an IDENTIFIER
                while (std::isalnum(peek())) advance();
                
                // Check if this identifier is actually a reserved keyword
                std::string text = source.substr(start, current - start);
                auto it = keywords.find(text);
                if (it != keywords.end()) {
                    addToken(it->second); // It's a keyword like 'let' or 'print'
                } else {
                    addToken(TokenType::IDENTIFIER); // It's just a variable name
                }
            } 
            else {
                std::cerr << "Error: Unexpected character '" << c << "' at line " << line << "\n";
            }
            break;
    }
}