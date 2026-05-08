
#include "compiler.h"
#include <iostream>
#include <string>

// --- Core Emitter Logic ---

void Compiler::emitByte(uint8_t byte) {
    chunk.code.push_back(byte);
}

void Compiler::emitOpcode(Opcode code) {
    // Cast our strongly-typed enum back to a raw byte
    emitByte(static_cast<uint8_t>(code));
}

// --- AST Traversal (The Flattening) ---

void Compiler::compileStatement(Stmt* stmt) {
    // dynamic_cast lets us check which specific Stmt node we are looking at
    if (auto* printStmt = dynamic_cast<PrintStmt*>(stmt)) {
        
        compileExpression(printStmt->expression.get());
        emitOpcode(Opcode::OP_PRINT); // Print expects a value to be on top of the stack
        
    } else if (auto* varStmt = dynamic_cast<VarStmt*>(stmt)) {
        
        if (varStmt->initializer) {
            compileExpression(varStmt->initializer.get());
        } else {
            // If the user just wrote 'let x;', we default it to 0
            int constIndex = chunk.addConstant(0);
            emitOpcode(Opcode::OP_CONSTANT);
            emitByte(constIndex);
        }
        
        // Store the variable name in our string pool
        int nameIndex = chunk.addString(varStmt->name.lexeme);
        emitOpcode(Opcode::OP_DEFINE_VAR);
        emitByte(nameIndex); // The operand for OP_DEFINE_VAR is the name's index
    }
    else if (auto* ifStmt = dynamic_cast<IfStmt*>(stmt)) {
        
        // 1. Compile the condition
        compileExpression(ifStmt->condition.get());

        // 2. Emit jump instruction with dummy offset
        int thenJump = emitJump(Opcode::OP_JUMP_IF_FALSE);

        // 3. Compile the "then" block
        compileStatement(ifStmt->thenBranch.get());

        // 4. If there's an 'else', we need to jump OVER it when 'then' finishes
        int elseJump = -1;
        if (ifStmt->elseBranch) {
            elseJump = emitJump(Opcode::OP_JUMP);
        }

        // 5. Backpatch the original JUMP_IF_FALSE to land exactly here!
        patchJump(thenJump);

        // 6. Compile the "else" block (if it exists) and backpatch the else jump
        if (ifStmt->elseBranch) {
            compileStatement(ifStmt->elseBranch.get());
            patchJump(elseJump);
        }

    } else if (auto* whileStmt = dynamic_cast<WhileStmt*>(stmt)) {
        
        // Record the starting IP of the loop so we know where to jump back to
        int loopStart = chunk.code.size();

        // Compile condition and setup exit jump
        compileExpression(whileStmt->condition.get());
        int exitJump = emitJump(Opcode::OP_JUMP_IF_FALSE);

        // Compile loop body
        compileStatement(whileStmt->body.get());

        // Emit an unconditional loop backwards to the start
        emitOpcode(Opcode::OP_LOOP);
        
        // Calculate how far back we need to jump (+1 to include the operand byte)
        int loopOffset = chunk.code.size() + 1 - loopStart;
        emitByte(static_cast<uint8_t>(loopOffset));

        // Backpatch the exit jump so it lands after the backwards loop instruction
        patchJump(exitJump);
    }
    else if (auto* blockStmt = dynamic_cast<BlockStmt*>(stmt)) {
        
        for (const auto& innerStmt : blockStmt->statements) {
            compileStatement(innerStmt.get());
        }
    }
    else if (auto* funcStmt = dynamic_cast<FunctionStmt*>(stmt)) {
        
        // 1. We emit a jump to skip the function body so it doesn't run when defined
        int jumpOver = emitJump(Opcode::OP_JUMP);
        
        // 2. We record EXACTLY where the body starts in the Chunk's memory
        chunk.functions[funcStmt->name.lexeme] = chunk.code.size();
        
        // 3. Magic Parameter Binding: 
        // The caller pushed arguments to the stack. We pop them into variables!
        // We do this in reverse order because the execution stack is LIFO.
        for (auto it = funcStmt->params.rbegin(); it != funcStmt->params.rend(); ++it) {
            int paramNameIdx = chunk.addString(it->lexeme);
            emitOpcode(Opcode::OP_DEFINE_VAR);
            emitByte(paramNameIdx);
        }

        // 4. Compile the inside of the function
        for (const auto& s : funcStmt->body) compileStatement(s.get());
        
        // 5. Safety Net: If a function doesn't have a 'return', we force it to return 0
        emitOpcode(Opcode::OP_CONSTANT); 
        emitByte(chunk.addConstant(0));
        emitOpcode(Opcode::OP_RETURN);
        
        // 6. Land the jump exactly here
        patchJump(jumpOver);

    } else if (auto* retStmt = dynamic_cast<ReturnStmt*>(stmt)) {
        if (retStmt->value) compileExpression(retStmt->value.get());
        else { 
            emitOpcode(Opcode::OP_CONSTANT); 
            emitByte(chunk.addConstant(0)); 
        }
        emitOpcode(Opcode::OP_RETURN);
    }
}

void Compiler::compileExpression(Expr* expr) {
    if (auto* binary = dynamic_cast<BinaryExpr*>(expr)) {
        
        // 1. Compile Left side
        compileExpression(binary->left.get());
        // 2. Compile Right side
        compileExpression(binary->right.get()); 

        // 3. Emit the Operator (Post-Order!)
        switch (binary->op.type) {
            case TokenType::PLUS:        emitOpcode(Opcode::OP_ADD); break;
            case TokenType::MINUS:       emitOpcode(Opcode::OP_SUBTRACT); break;
            case TokenType::STAR:        emitOpcode(Opcode::OP_MULTIPLY); break;
            case TokenType::SLASH:       emitOpcode(Opcode::OP_DIVIDE); break;
            case TokenType::EQUAL_EQUAL: emitOpcode(Opcode::OP_EQUAL); break;
            case TokenType::LESS:        emitOpcode(Opcode::OP_LESS); break;
            default: break;
        }
        
    } else if (auto* literal = dynamic_cast<LiteralExpr*>(expr)) {
        
        if (literal->value.type == TokenType::NUMBER) {
            int val = std::stoi(literal->value.lexeme);
            int index = chunk.addConstant(val); // Put number in the pool
            emitOpcode(Opcode::OP_CONSTANT);    // Tell VM to load a constant
            emitByte(index);                    // Tell VM WHICH constant to load
        } else if (literal->value.type == TokenType::TRUE_LITERAL) {
            int index = chunk.addConstant(1);
            emitOpcode(Opcode::OP_CONSTANT);
            emitByte(index);
        } else if (literal->value.type == TokenType::FALSE_LITERAL) {
            int index = chunk.addConstant(0);
            emitOpcode(Opcode::OP_CONSTANT);
            emitByte(index);
        }
        
    } else if (auto* variable = dynamic_cast<VariableExpr*>(expr)) {
        
        int nameIndex = chunk.addString(variable->name.lexeme);
        emitOpcode(Opcode::OP_GET_VAR);
        emitByte(nameIndex);
        
    }else if (auto* callExpr = dynamic_cast<CallExpr*>(expr)) {
        // 1. Push all arguments onto the stack first
        for (auto& arg : callExpr->arguments) {
            compileExpression(arg.get());
        }
        
        // 2. Emit the CALL instruction with the function's name
        int nameIdx = chunk.addString(callExpr->callee.lexeme);
        emitOpcode(Opcode::OP_CALL);
        emitByte(nameIdx);
    }
    else if (dynamic_cast<InputExpr*>(expr)) {
        // When we see the 'input' keyword, just emit the instruction!
        emitOpcode(Opcode::OP_INPUT);
    }
}