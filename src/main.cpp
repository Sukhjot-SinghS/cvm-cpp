#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#include <chrono> 
// Notice: <iomanip> is completely gone!

#include "lexer.h"
#include "parser.h"
#include "compiler.h"
#include "vm.h"

// 👇 --- ANSI COLOR CODES FOR TERMINAL FLEX --- 👇
#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define GREEN   "\033[32m"
#define CYAN    "\033[36m"
#define YELLOW  "\033[33m"
#define MAGENTA "\033[35m"
#define RED     "\033[31m"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << RED << "Usage: ./cv <script.cvm>\n" << RESET;
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << RED << "Could not open file " << argv[1] << "\n" << RESET;
        return 1;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    std::cout << BOLD << CYAN << "\n=== [ CVM++ CORE PIPELINE INITIATED ] ===\n" << RESET;

    try {
        // 1. Lexer
        std::cout << YELLOW << "[1/4] Running Lexical Analysis... " << RESET;
        Lexer lexer(source);
        std::vector<Token> tokens = lexer.scanTokens();
        std::cout << GREEN << "Done. (" << tokens.size() << " tokens generated)\n" << RESET;

        // 2. Parser
        std::cout << YELLOW << "[2/4] Parsing Context-Free Grammar (AST)... " << RESET;
        Parser parser(tokens);
        auto statements = parser.parse();
        std::cout << GREEN << "Done.\n" << RESET;
        
        // 3. Compiler
        std::cout << YELLOW << "[3/4] Compiling to Flat Bytecode... " << RESET;
        Compiler compiler;
        Chunk chunk = compiler.compile(statements);
        std::cout << GREEN << "Done.\n" << RESET;

        // 👇 --- BULLETPROOF BYTECODE DISASSEMBLER --- 👇
        std::cout << "\n" << BOLD << MAGENTA << "=== [ BYTECODE DISASSEMBLER ] ===" << RESET << "\n";
        
        for (size_t i = 0; i < chunk.code.size(); i++) {
            uint8_t inst = chunk.code[i];
            
            // 1. Manual Zero-Padding for Index (e.g., turns "2" into "002")
            std::string idxStr = std::to_string(i);
            while (idxStr.length() < 3) idxStr = "0" + idxStr;
            std::cout << CYAN << "[" << idxStr << "] " << RESET;
            
            // 2. Determine Opcode Name and Args
            std::string opName;
            
            if (inst == static_cast<uint8_t>(Opcode::OP_CONSTANT)) {
                opName = "OP_CONSTANT";
                while (opName.length() < 18) opName += " "; // Manual space padding
                std::cout << opName << YELLOW << "(Value: " << chunk.constants[chunk.code[++i]] << ")" << RESET << "\n";
                
            } else if (inst == static_cast<uint8_t>(Opcode::OP_JUMP_IF_FALSE) || 
                       inst == static_cast<uint8_t>(Opcode::OP_JUMP)) {
                opName = (inst == static_cast<uint8_t>(Opcode::OP_JUMP)) ? "OP_JUMP" : "OP_JUMP_IF_FALSE";
                while (opName.length() < 18) opName += " ";
                std::cout << opName << YELLOW << "(Offset: +" << (int)chunk.code[++i] << " bytes)" << RESET << "\n";
                
            } else if (inst == static_cast<uint8_t>(Opcode::OP_CALL)) {
                opName = "OP_CALL";
                while (opName.length() < 18) opName += " ";
                std::cout << opName << GREEN << "(Function: " << chunk.strings[chunk.code[++i]] << ")" << RESET << "\n";
                
            } else if (inst == static_cast<uint8_t>(Opcode::OP_RETURN)) {
                std::cout << "OP_RETURN\n";
                
            } else if (inst == static_cast<uint8_t>(Opcode::OP_DEFINE_VAR) || inst == static_cast<uint8_t>(Opcode::OP_GET_VAR)) {
                opName = (inst == static_cast<uint8_t>(Opcode::OP_DEFINE_VAR)) ? "OP_DEFINE_VAR" : "OP_GET_VAR";
                while (opName.length() < 18) opName += " ";
                std::cout << opName << CYAN << "(Name: " << chunk.strings[chunk.code[++i]] << ")" << RESET << "\n";
                
            } else if (inst == static_cast<uint8_t>(Opcode::OP_INPUT)) {
                std::cout << "OP_INPUT\n";
                
            } else {
                std::cout << "OP_MATH/LOGIC\n"; 
            }
        }
        std::cout << BOLD << MAGENTA << "=================================" << RESET << "\n\n";
        // 👆 ---------------------------------------------- 👆

        // 4. Virtual Machine CPU
        std::cout << YELLOW << "[4/4] Executing Virtual Machine CPU...\n" << RESET;
        VM vm;

        // START HIGH-PRECISION TIMER
        auto start_time = std::chrono::high_resolution_clock::now();
        
        vm.interpret(&chunk);
        
        // STOP TIMER AND CALCULATE
        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> ms_double = end_time - start_time;
        
        std::cout << "\n" << BOLD << GREEN << "=======================================" << RESET << "\n";
        std::cout << BOLD << " ⏱️  VM Execution Time: " << YELLOW << ms_double.count() << " ms\n" << RESET;
        std::cout << BOLD << GREEN << "=======================================" << RESET << "\n\n";
        
    } catch (const std::exception& e) {
        std::cerr << "\n" << BOLD << RED << "[FATAL SYSTEM CRASH] " << e.what() << "\nExecution Halted.\n" << RESET;
        return 1;
    }

    return 0;
}