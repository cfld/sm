CXXFLAGS += -std=c++11 -mtune=native -march=native -Wall -O3 -DNDEBUG -g -fopenmp
LIBS += -lrt

all: vf2

vf2: vf2.cpp
	g++ $(CXXFLAGS) -o bin/vf2 vf2.cpp

clean:
	rm -rf bin/* *.o
