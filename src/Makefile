# --- CVM++ Build System ---

# 1. Compiler and Flags
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17

# 2. Target Executable Name
# Note: Generates 'cv' on Linux/Mac, and 'cv.exe' on Windows automatically
TARGET = cvm

# 3. Source and Object Files
SRCS = main.cpp lexer.cpp parser.cpp compiler.cpp vm.cpp
OBJS = $(SRCS:.cpp=.o)

# 4. Default build rule (runs when you type 'make')
all: $(TARGET)

# 5. Rule to link object files into the final executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# 6. Rule to compile individual .cpp files into .o (object) files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 7. Quick command to build AND run the code immediately
run: $(TARGET)
	./$(TARGET) test.cvm

# 8. Clean command to remove compiled files
clean:
	rm -f $(OBJS) $(TARGET) $(TARGET).exe
