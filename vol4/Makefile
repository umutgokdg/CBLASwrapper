CC=gcc
CXX=g++
CFLAGS=-Wall -g
CXXFLAGS=-Wall -g
LDFLAGS=

# Define lists of files
CFILES := $(wildcard CFiles/*.c)
CPPFILES := $(wildcard CppFiles/*.cpp)
HFILES := $(wildcard HFiles/*.h)
HPPFILES := $(wildcard HppFiles/*.hpp)

# Convert source files to object files by replacing suffixes
COBJS := $(CFILES:.c=.o)
CPPOBJS := $(CPPFILES:.cpp=.o)

# Define the executables and objects
EXEC=main
OBJS=$(COBJS) $(CPPOBJS) main.o

# Rule for linking the final executable
$(EXEC): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^

# Rule for compiling C source files
$(COBJS): %.o: %.c $(HFILES)
	$(CC) $(CFLAGS) -c $< -o $@

# Rule for compiling C++ source files
$(CPPOBJS): %.o: %.cpp $(HPPFILES)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Rule for cleaning up generated files
clean:
	rm -f $(EXEC) $(OBJS)

# Dependencies are automatically managed by the implicit rules
