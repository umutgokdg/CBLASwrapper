CC = gcc
CXX = g++
CFLAGS = -Wall -Wextra -std=c11
CXXFLAGS = -Wall -Wextra -std=c++11

# List of header file directories
HEADER_DIRS = tempdir/temppdir

# Convert header directories to -I flags
HEADER_FLAGS = $(addprefix -I ,$(HEADER_DIRS))

C_SOURCES = CFiles/baseclass.c
CPP_SOURCES = trying.cpp
C_OBJECTS = $(C_SOURCES:.c=.o)
CPP_OBJECTS = $(CPP_SOURCES:.cpp=.o)
TARGET = program

all: $(TARGET)

$(TARGET): $(C_OBJECTS) $(CPP_OBJECTS)
	$(CXX) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(HEADER_FLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(HEADER_FLAGS) -c $< -o $@

clean:
	rm -f $(C_OBJECTS) $(CPP_OBJECTS) $(TARGET)

run_and_clean: $(TARGET)
	./$(TARGET)
	@$(MAKE) clean
