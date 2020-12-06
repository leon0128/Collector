CXX = g++-9 
CXXFLAGS = -std=c++17 -w -g3
PROGRAM = collector
DIR = src
OBJS = $(patsubst %.cpp, %.o, $(wildcard $(DIR)/*.cpp))

$(PROGRAM): $(OBJS)
	$(CXX) $(OBJS) $(CXXFLAGS) -o $(PROGRAM)

clean:
	rm -f $(DIR)/*.o $(PROGRAM)
