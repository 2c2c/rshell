CXX=g++
CXXFLAGS= -Wall -Werror -ansi -pedantic -std=c++11 -g

all: bin rshell ls cp

bin: 
	[ -e bin ] || mkdir bin

rshell:
	$(CXX) $(CXXFLAGS) ./src/rshell.cpp -o bin/rshell
ls:
	$(CXX) $(CXXFLAGS) ./src/ls.cpp -o bin/ls
cp:
	$(CXX) $(CXXFLAGS) ./src/cp.cpp -o bin/cp

clean:
	rm -rf bin
