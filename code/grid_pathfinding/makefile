CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra

TARGET := grid_pathfinding
SRCS := grid_pathfinding.cpp \
        algorithms/brute_force.cpp \
        algorithms/dynamic_programming.cpp \
        algorithms/greedy.cpp
OBJS := $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean