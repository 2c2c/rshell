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
#include <set>
#include <ctype.h>
// puts the argv input from the program into a list
std::list<std::string> GetInput(int argc, char **argcv);

// outputs parameters to the program for debugging
void Output(std::list<std::string> input);

// grabbed from UserHostInfo from rshell.cpp
// the gains from refactoring aren't worth bothering
std::string CurrentPwd();

// iterate each character of each element with a - to look for unknown args
bool UnknownArgs(std::list<std::string> input);
std::set<std::string> Split(std::list<std::string> input);
int main(int argc, char **argv) {
  auto input = GetInput(argc, argv);
  Output(input);
  if(UnknownArgs(input))
    std::cout << "oh no" << std::endl;

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
std::set<std::string> Split(std::list<std::string> input) {
  std::set<std::string> args;
  input.pop_front();
  for (auto& item : input) {
    if (item[0] == '-') {
      item.erase(item.begin());
      for(const auto& letter : item) {
        args.emplace(letter);
        }
    }
  }
  return args;
}
bool UnknownArgs(std::list<std::string> input) {
  for (auto &item : input) {
    if (item[0] == '-') {
      item.erase(item.begin());
      for (const auto &letter : item) {
        if (letter != 'l' && letter != 'a' && tolower(letter) != 'r')
          return true;
      }
    }
  }
  return false;
}
