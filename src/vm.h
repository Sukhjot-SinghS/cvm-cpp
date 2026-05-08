
#ifndef VM_H
#define VM_H

#include "compiler.h"
#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>
class VM {
private:
    Chunk* chunk;
    int ip; // Instruction Pointer (like your Program Counter)
    
    std::vector<int> stack; // The heart of the machine
    std::unordered_map<std::string, int> globals; // Our RAM for variables
    std::vector<int> callStack;
    std::vector<std::unordered_map<std::string, int>> scopeStack;

    // Core CPU mechanisms
    void push(int value);
    int pop();
    int getVariable(const std::string& name);
    
    // Instruction Fetch mechanisms
    uint8_t readByte();
    int readConstant();
    std::string readString();

public:
    VM() : chunk(nullptr), ip(0) {}

    // Kicks off the execution loop
    void interpret(Chunk* compiledChunk);
};

#endif // VM_H