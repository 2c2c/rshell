CXX=g++
CXXFLAGS= -Wall -Werror -ansi -pedantic -std=c++11 -g

all: bin rshell ls

bin: 
	[ -e ./bin ] || mkdir bin

rshell:
	$(CXX) $(CXXFLAGS) ./src/rshell.cpp -o ./bin/rshell
ls:
	$(CXX) $(CXXFLAGS) ./src/ls.cpp -o ./bin/ls

clean:
	rm -rf bin
