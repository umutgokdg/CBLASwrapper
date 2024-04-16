CC=gcc
CXX=g++
CFLAGS=-Wall -g
CXXFLAGS=-Wall -g
LDFLAGS=

# Define the executables and objects
EXEC=main
OBJS=CFiles/c_library.o CppFiles/point_wrapper.o main.o

# Rule for linking the final executable
$(EXEC): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^

# Rule for compiling C source files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Rule for compiling C++ source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Rule for cleaning up generated files
clean:
	rm -f $(EXEC) $(OBJS)

# Dependencies
PointWrapper.o: HppFiles/point_wrapper.hpp c_library.h
main.o: HppFiles/point_wrapper.hpp
c_library.o: c_library.h
