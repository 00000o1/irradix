# Makefile for the Irradix C++ Project

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -pedantic -O2
CXXFLAGS += -isysroot $(shell xcrun --show-sdk-path)


# Targets and sources
TARGET = irradix_tool
SOURCES = main.cpp
HEADERS = irradix.hpp
OBJECTS = $(SOURCES:.cpp=.o)

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS)

# Compile object files
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	rm -f $(OBJECTS) $(TARGET) ? *.txt *.bin

# Run the tool with some default arguments (example usage)
run: $(TARGET)
	./$(TARGET) encode 42

.PHONY: all clean run

