#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <string>
#include <boost/tokenizer.hpp>
#include <vector>
#include <list>

// puts the argv input from the program into a list
std::list<std::string> GetInput(int argc, char **argcv);

// outputs parameters to the program for debugging
void Output(std::list<std::string> input);

// grabbed from UserHostInfo from rshell.cpp
// the gains from refactoring aren't worth bothering
std::string CurrentPwd();

bool UnknownArgs(const std::list<std::string> input);
std::list<std::string> Split(std::string args);
int main(int argc, char **argv) {
  auto input = GetInput(argc, argv);
  Output(input);

  //    DIR *test = opendir("test");
  //    readdir(test);
  //    struct stat *buf= new struct stat;
  //    stat("test",buf);
  //    buf->st_gid
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
void Output(std::list<std::string> input) {
  for (const auto &element : input)
    std::cout << element << std::endl;
}

std::string CurrentPwd() {
  char *rawpwd = get_current_dir_name();
  if (rawpwd == NULL) {
    perror("get_current_dir_name returned NULL");
    exit(1);
  }
  std::string pwd(rawpwd);
  delete rawpwd;
  return pwd;
}
std::list<std::string> Split(std::string args) {
  std::string foo = args;
  std::list<std::string> test;
  return test;
}
bool UnknownArgs(const std::list<std::string> input) {
  input.empty();
  return true;
}
