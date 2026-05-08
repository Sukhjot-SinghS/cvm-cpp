# ⚡ CVM++ : Custom Virtual Machine & Bytecode Compiler

**CVM++** is a lightweight, stack-based Virtual Machine and Bytecode Compiler built entirely from scratch in modern C++17. 

It parses a custom, high-level scripting language into an Abstract Syntax Tree (AST), flattens it into a custom 1D bytecode Instruction Set Architecture (ISA), and executes it via a high-performance virtual CPU. **No external parser generators (like Flex or Bison) were used.**

---

## 🧠 Core Architecture Pipeline

The engine translates human-readable scripts into machine execution through a strict 4-phase pipeline:

1. **Lexical Analyzer (Scanner):** Converts raw string source code into a stream of categorized `Tokens` (Identifiers, Keywords, Operators).
2. **Recursive Descent Parser:** Consumes tokens to enforce grammar rules and builds a deeply nested Abstract Syntax Tree (AST).
3. **Bytecode Compiler:** Walks the AST and flattens it into a 1D array of raw Bytecode, utilizing forward-jumping offsets for control flow and loops.
4. **Stack-Based Virtual Machine:** A simulated CPU featuring an Instruction Pointer (`ip`), an Execution Stack, a Scope Stack (for memory isolation), and a Call Stack (to support recursive function calls).

---

## 🛠️ Getting Started (Build Instructions)

CVM++ uses a standard `Makefile` for zero-friction compilation. You only need `g++` (GCC) installed on your system.

### 1. Clone & Compile
```bash
git clone https://github.com/yourusername/cvm-plus-plus.git
cd cvm-plus-plus/src
make
```
(This will generate the cv executable in your directory).

### 2. Run a Script
Create a file named `test.cvm`, write your code, and execute it:

```bash
./cv test.cvm
```
(Note for Windows users: The Makefile automatically generates `cv.exe`. Run `./cv.exe test.cvm`)

---

## 🧪 Interactive Test Suite
Want to see what the engine can do? Copy and paste any of these test cases into a `test.cvm` file and run it!

### Test Case 1: Variables & Interactive I/O
Testing lexical scoping and the native VM halt/resume mechanism.

```javascript
let base = 100
print 88888 // Prompt indicator
let multiplier = input

def calculate(a, b) {
    let result = a * b
    return result
}

let final_answer = calculate(base, multiplier)
print final_answer
```

### Test Case 2: Control Flow & Loops
Testing the Compiler's bytecode offset patching (`OP_JUMP` and `OP_JUMP_IF_FALSE`).

```javascript
let count = 0
let limit = 5

while count < limit {
    if count < 3 {
        print 0 // Prints 0 for the first three iterations
    } else {
        print 1 // Prints 1 for the remaining iterations
    }
    let count = count + 1
}
```

### Test Case 3: The Final Boss (Deep Recursion)
Testing the Virtual Machine's Call Stack and Activation Records. This calculates the 10th number in the Fibonacci sequence.

```javascript
def fibonacci(n) {
  if n < 2 {
    return n
  }
  return fibonacci(n - 1) + fibonacci(n - 2)
}

let ans = fibonacci(10)
print ans // Expected output: 55
```

---

## 🛡️ Error Handling & Safety
CVM++ is designed to fail gracefully without causing C++ hardware-level panics (like SIGFPE or Segfaults).

**Compile-Time Safety:** The Recursive Descent Parser catches illegal grammar (e.g., missing brackets) and halts before bytecode generation.

**Runtime Safety:** The VM CPU natively intercepts impossible math (Division by Zero), Memory Underflows, and Undefined Variable Access, throwing formatted exceptions to the terminal.

---

## 🚀 Future Roadmap
- [ ] Heap Allocation for dynamically sized types.
- [ ] Native String support ("Hello World").
- [ ] Arrays and Garbage Collection.
- [ ] Standard Library bindings (C++ math and time functions).
