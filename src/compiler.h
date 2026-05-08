
#ifndef COMPILER_H
#define COMPILER_H

#include "parser.h"
#include <vector>
#include <string>
#include <cstdint>
#include <unordered_map>

// 1. Our Custom Instruction Set Architecture (ISA)
enum class Opcode : uint8_t {
    OP_CONSTANT,    // Push a number onto the stack
    OP_ADD,         // Add top two numbers
    OP_SUBTRACT,    // Subtract top two numbers
    OP_MULTIPLY,    // Multiply top two numbers
    OP_DIVIDE,      // Divide top two numbers
    OP_EQUAL,       // Check if top two are equal
    OP_LESS,        // Check if second-to-top is less than top
    OP_DEFINE_VAR,  // Store top of stack into a variable
    OP_GET_VAR,     // Push a variable's value onto the stack
    OP_PRINT,       // Pop and print the top of the stack
    OP_RETURN,      // End of program
    OP_JUMP_IF_FALSE, // Jump over the block if condition is 0
    OP_JUMP,          // Unconditional jump (used for loops and else blocks)
    OP_LOOP,
    OP_CALL, 
    OP_INPUT   // Teleport the IP to a function

};

// 2. A "Chunk" represents our compiled program block
struct Chunk {
    std::vector<uint8_t> code;
    std::vector<int> constants;
    std::vector<std::string> strings;
    
    // NEW: A map to remember at exactly which byte index a function starts!
    std::unordered_map<std::string, int> functions; 

    int addConstant(int value) { constants.push_back(value); return constants.size() - 1; }
    int addString(const std::string& str) { strings.push_back(str); return strings.size() - 1; }
};
    // Helper to add a constant and return its index



// 3. The Compiler Class
class Compiler {
private:
    Chunk chunk;

    // --- Core Emitter logic ---
    void emitByte(uint8_t byte);
    void emitOpcode(Opcode code);

    int emitJump(Opcode instruction) {
    emitOpcode(instruction);
    emitByte(0xff); // Push a dummy 8-bit offset (255) for now
    return chunk.code.size() - 1; // Return the exact index of this dummy byte
    }
    void patchJump(int offsetIndex) {
    // Calculate how many bytes we generated AFTER the dummy byte
    int jumpLength = chunk.code.size() - 1 - offsetIndex;
    
    if (jumpLength > 255) {
        std::cerr << "Error: Jump body too large for 8-bit offset!\n";
    }
    
    // Go back in time and overwrite the dummy byte with the real distance
    chunk.code[offsetIndex] = static_cast<uint8_t>(jumpLength);
}

    // --- AST Traversal logic ---
    void compileStatement(Stmt* stmt);
    void compileExpression(Expr* expr);

public:
    Compiler() = default;

    // Entry point: takes the AST and returns the compiled bytecode chunk
    Chunk compile(const std::vector<std::unique_ptr<Stmt>>& statements) {
        for (const auto& stmt : statements) {
            compileStatement(stmt.get());
        }
        emitOpcode(Opcode::OP_RETURN);
        return chunk;
    }
};

#endif // COMPILER_H