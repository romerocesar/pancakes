CXX=g++
CXXFLAGS=-O3 -DNDEBUG
SOURCES=main.cpp pancake.cpp search.cpp getopt_pp.cpp 
TEST=pancake_test.cpp pancake.o search.o
OBJECTS=$(SOURCES:.cpp=.o)
BINARY=pancakes

all: $(OBJECTS) $(BINARY)

test: CXXFLAGS=-g3 -O0
test: $(TEST)
	$(CXX) $(CXXFLAGS) $(TEST) -o $@

$(BINARY): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $< -o $@

clean:
	rm search.o pancake.o main.o pancakes