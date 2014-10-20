CXX=g++
CXXFLAGS= -Wall -Werror -ansi -pedantic -std=c++11

all: bin rshell

bin: 
	[ -e ./bin ] || mkdir bin

rshell:
	$(CXX) $(CXXFLAGS) ./src/rshell.cpp -o ./bin/rshell

clean:
	rm -rf bin
