#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <list>
#include <set>
#include <stdio.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "Timer.h"

std::list<std::string> GetInput(int argc, char **argv);

void cp_fstream(std::list<std::string> input);
void cp_unixsingle(std::list<std::string> input);
void cp_unixbuffer(std::list<std::string> input);

void Benchmark(std::list<std::string> input);

bool ArgSizeError(int num_args);

std::set<std::string> Split(std::list<std::string> input);
bool UnknownArgs(std::list<std::string> input);

int main(int argc, char **argv) {
  auto input = GetInput(argc, argv);
  if (ArgSizeError(argc)) {
    std::cout << "improper number of arguments" << std::endl;
    exit(1);
  }
  if (UnknownArgs(input)) {
    std::cout << "unknown flag used" << std::endl;
    exit(1);
  }
  auto args = Split(input);
  // if no optional arg, do the most efficient cp (buffer)
  if (args.empty()) {
    cp_unixbuffer(input);
  } else {
    // otherwise do the benchmark but cleanup the dasharg from the parameter
    // list
    // since dasharg could be anywhere in the list it gets rid of ambiguity
    // of which elements in the list are the file directories
    auto dasharg = input.begin();
    while (dasharg != input.end()) {
      if (dasharg->front() == '-')
        break;
      dasharg++;
    }

    input.erase(dasharg);
    Benchmark(input);
  }
}
std::list<std::string> GetInput(int argc, char **argv) {
  using namespace std;
  list<string> input;
  for (int i = 0; i < argc; i++) {
    string piece(argv[i]);
    input.push_back(piece);
  }
  return input;
}
void cp_fstream(std::list<std::string> input) {
  using namespace std;
  input.pop_front();

  string filename = input.front();
  ifstream fin;
  fin.open(filename);
  if (!fin.is_open()) {
    cout << "fin isn't open" << endl;
    exit(1);
  }

  input.pop_front();

  string destination = input.front();

  access(destination.c_str(), W_OK);
  if (errno != ENOENT) {
    perror("file either already exists or another error occurred\n");
    exit(1);
  }

  ofstream fout;
  fout.open(destination);
  if (!fout.is_open()) {
    cout << "fout isn't open" << endl;
    exit(1);
  }
  while (fin.good()) {
    char c = fin.get();
    if (fin.good()) {
      fout.put(c);
    }
  }
}
void cp_unixsingle(std::list<std::string> input) {
  using namespace std;
  input.pop_front();

  string filename = input.front();
  input.front();
  cout << filename << endl;
  auto readfd = open(input.front().c_str(), O_RDONLY, 0);
  input.pop_front();
  // error check
  if (errno != 0) {
    perror("openr error");
    exit(1);
  }
  char buffer;
  int num;
  string destination = input.front();
  auto writefd = open(input.front().c_str(), O_WRONLY | O_CREAT | O_EXCL,
                      S_IREAD | S_IWRITE);
  // error check
  if (errno != 0) {
    perror("openw error");
    exit(1);
  }
  while ((num = read(readfd, &buffer, 1)) > 0) {
    // error check
    if (errno != 0) {
      perror("read error");
      exit(1);
    }
    write(writefd, &buffer, 1);
    if (errno != 0) {
      perror("write error");
      exit(1);
    }
  }
}
void cp_unixbuffer(std::list<std::string> input) {
  using namespace std;
  input.pop_front();

  string filename = input.front();
  input.front();
  cout << filename << endl;
  auto readfd = open(input.front().c_str(), O_RDONLY, 0);
  // error check
  if (errno != 0) {
    perror("openr error");
    exit(1);
  }
  char buffer[BUFSIZ];
  int num;
  input.pop_front();
  string destination = input.front();
  auto writefd = open(input.front().c_str(), O_WRONLY | O_CREAT | O_EXCL,
                      S_IREAD | S_IWRITE);
  // error check
  if (errno != 0) {
    perror("openw error");
    exit(1);
  }
  while ((num = read(readfd, &buffer, BUFSIZ)) > 0) {
    // error check
    if (errno != 0) {
      perror("read error");
      exit(1);
    }
    write(writefd, &buffer, BUFSIZ);
    if (errno != 0) {
      perror("write error");
      exit(1);
    }
  }
}
void Benchmark(std::list<std::string> input) {
  using namespace std;

  Timer t1;
  double elapsed_time;
  auto destination = input.begin();
  destination++;
  destination++;
  t1.start();
  cp_fstream(input);
  t1.elapsedUserTime(elapsed_time);
  cout << "fstream: " << elapsed_time << endl;
  remove(destination->c_str());
  if (errno != 0) {
    perror("error removing benchmark copy");
    exit(1);
  }
  Timer t2;
  t2.start();
  cp_unixsingle(input);
  t2.elapsedUserTime(elapsed_time);
  cout << "unixsingle: " << elapsed_time << endl;
  remove(destination->c_str());
  if (errno != 0) {
    perror("error removing benchmark copy");
    exit(1);
  }
  Timer t3;
  t3.start();
  cp_unixbuffer(input);
  t3.elapsedUserTime(elapsed_time);
  cout << "unixbuffer: " << elapsed_time << endl;
  remove(destination->c_str());
  if (errno != 0) {
    perror("error removing benchmark copy");
    exit(1);
  }
}
bool ArgSizeError(int argc) {
  if (argc != 3 || argc != 4)
    return false;
  return true;
}

std::set<std::string> Split(std::list<std::string> input) {
  std::set<std::string> args;
  input.pop_front();
  for (auto &item : input) {
    if (item[0] == '-') {
      item.erase(item.begin());
      for (const auto &letter : item) {
        args.insert(std::string(1, tolower(letter)));
      }
    }
  }
  return args;
}
bool UnknownArgs(std::list<std::string> input) {
  for (auto &item : input) {
    if (item[0] == '-') {
      item.erase(item.begin());
      if (item.empty())
        return true;
      for (const auto &letter : item) {
        if (letter != 'b')
          return true;
      }
    }
  }
  return false;
}
