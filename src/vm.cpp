
#include "vm.h"
#include <iostream>
#include <stdexcept>

// --- Stack Operations ---
void VM::push(int value) {
    stack.push_back(value);
}

int VM::pop() {
    if (stack.empty()) {
        throw std::runtime_error("\n[Runtime Error] Stack Underflow! Trying to pop from an empty stack.");
    }
    int value = stack.back();
    stack.pop_back();
    return value;
}

int VM::getVariable(const std::string& name) {
    if (scopeStack.back().count(name)) return scopeStack.back()[name];
    if (scopeStack.front().count(name)) return scopeStack.front()[name];
    throw std::runtime_error("\n[Runtime Error] Undefined variable: '" + name + "'");
}

// --- Fetch Operations ---
uint8_t VM::readByte() {
    return chunk->code[ip++]; // Read the byte, then increment the Program Counter!
}

int VM::readConstant() {
    return chunk->constants[readByte()]; // Read the operand byte, use it as an index
}

std::string VM::readString() {
    return chunk->strings[readByte()];
}

// --- The Core Loop ---
void VM::interpret(Chunk* compiledChunk) {
    chunk = compiledChunk;
    ip = 0; // Reset Program Counter to start
    scopeStack.clear();
    scopeStack.push_back({}); // Create the Global Scope
    callStack.clear();

    while (true) {
        uint8_t instruction = readByte();

        switch (static_cast<Opcode>(instruction)) {
            case Opcode::OP_CONSTANT: {
                int constant = readConstant();
                push(constant);
                break;
            }
            case Opcode::OP_ADD: {
                int b = pop(); // Right operand is popped first!
                int a = pop(); // Left operand is popped second!
                push(a + b);
                break;
            }
            case Opcode::OP_SUBTRACT: {
                int b = pop();
                int a = pop();
                push(a - b);
                break;
            }
            case Opcode::OP_MULTIPLY: {
                int b = pop();
                int a = pop();
                push(a * b);
                break;
            }
            case Opcode::OP_DIVIDE: {
                int b = pop();
                int a = pop();
                if (b == 0) throw std::runtime_error("\n[Runtime Error] Division by zero is not allowed!");
                push(a / b);
                break;
            }
            case Opcode::OP_EQUAL: {
                int b = pop();
                int a = pop();
                push(a == b ? 1 : 0); // 1 for true, 0 for false
                break;
            }
            case Opcode::OP_LESS: {
                int b = pop();
                int a = pop();
                push(a < b ? 1 : 0);
                break;
            }
            case Opcode::OP_DEFINE_VAR: {
                std::string name = readString();
                // Always define variables in the CURRENT local scope
                scopeStack.back()[name] = pop(); 
                break;
            }
            case Opcode::OP_GET_VAR: {
                std::string name = readString();
                push(getVariable(name));
                break;
            }
            case Opcode::OP_PRINT: {
                std::cout << pop() << "\n";
                break;
            }
            case Opcode::OP_JUMP_IF_FALSE: {
                uint8_t offset = readByte(); // Read the jump distance
                int condition = pop();       // Pop the evaluated condition
                
                if (condition == 0) {
                    ip += offset; // BRANCH TAKEN: Skip the `ip` forward!
                }
                break;
            }
            case Opcode::OP_JUMP: {
                uint8_t offset = readByte();
                ip += offset; // UNCONDITIONAL BRANCH: Always skip forward
                break;
            }
            case Opcode::OP_LOOP: {
                uint8_t offset = readByte();
                ip -= offset; // LOOP: Move the `ip` backward!
                break;
            }
            case Opcode::OP_RETURN: {
                if (callStack.empty()) return; // If call stack is empty, the whole script is finished!
                
                int retVal = pop();          // Save the return value
                scopeStack.pop_back();       // DESTROY the local variables
                ip = callStack.back();       // Fetch the return address
                callStack.pop_back();        // Remove it from the stack
                push(retVal);                // Put the return value back on the stack for the caller
                break;
                return; // Halt the machine!
            }
            case Opcode::OP_CALL: {
                std::string name = readString();
                
                // 1. Save where we currently are so we can return later
                callStack.push_back(ip); 
                
                // 2. Create a fresh, empty Local Memory space
                scopeStack.push_back({}); 
                
                // 3. Teleport!
                ip = chunk->functions[name]; 
                break;
            }
            case Opcode::OP_INPUT: {
                int value;
                std::cout << "> "; // Print a little prompt indicator
                std::cin >> value;
                
                // Clear the input buffer just in case the user typed weird stuff
                std::cin.clear();
                std::cin.ignore(10000, '\n'); 
                
                push(value); // Put the user's number onto the stack!
                break;
            }
        
        }
    }
}