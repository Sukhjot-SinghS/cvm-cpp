#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#include "lexer.h"
#include "parser.h"
#include "compiler.h"
#include "vm.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./cv.exe <script.cvm>\n";
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << "Could not open file " << argv[1] << "\n";
        return 1;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    // 1. Lexer
    Lexer lexer(source);
    std::vector<Token> tokens = lexer.scanTokens();

    // 2. Parser
    Parser parser(tokens);
    try {
        auto statements = parser.parse();
        Compiler compiler;
        Chunk chunk = compiler.compile(statements);
        VM vm;
        vm.interpret(&chunk);
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\nExecution Halted.\n";
        return 1;
    }

    return 0;
}