#include <algorithm>
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
#include <map>

// puts the argv input from the program into a list
std::list<std::string> GetInput(int argc, char **argcv);
void StripDotfiles(std::list<std::string> &names);

// outputs parameters to the program for debugging
void OutputElements(std::list<std::string> input);
void OutputArgs(std::set<std::string> args);

// grabbed from UserHostInfo from rshell.cpp
// the gains from refactoring aren't worth bothering
std::string CurrentPwd();

// iterate each character of each element with a - to look for unknown args
bool UnknownArgs(std::list<std::string> input);

std::set<std::string> Split(std::list<std::string> &input);

int main(int argc, char **argv) {
  using namespace std;
  auto input = GetInput(argc, argv);
  OutputElements(input);
  if (UnknownArgs(input))
    cout << "oh no" << endl;
  auto args = Split(input);
  OutputArgs(args);

  OutputElements(input);

  struct stat *buf = new struct stat;
  stat("test", buf);

  /*
   * This is a BARE BONES example of how to use opendir/readdir/closedir. Notice
   * that there is no error checking on these functions.  You MUST add error
   * checking yourself.
   */

  std::string dirName = ".";
  list<string> names;
  DIR *dirp = opendir(dirName.c_str());
  dirent *direntp;
  while ((direntp = readdir(dirp))) {
    names.push_back(direntp->d_name);
    std::cout << direntp->d_name
              << std::endl; // use stat here to find attributes of file
  }
  closedir(dirp);

  sort(names.begin(), names.end(), std::locale("en_US.UTF-8"));

  auto argcheck = args.find("a");
  if (argcheck == args.end())
    StripDotfiles(names);

  bool longlist;
  argcheck = args.find("l");
  if (argcheck == args.end()) {
    longlist = false;
  } else {
    longlist = true;
  }
  argcheck = args.find("R");
  if (argcheck == args.end()) {

  } else {
    // recursive
  }

  std::cout << "excuse me" << std::endl;

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
void OutputElements(std::list<std::string> input) {
  using namespace std;
  cout << "inputs: " << endl;
  for (const auto &element : input)
    cout << element << endl;
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
std::set<std::string> Split(std::list<std::string> &input) {
  std::set<std::string> args;
  input.pop_front();
  for (auto &item : input) {
    if (item[0] == '-') {
      item.erase(item.begin());
      for (const auto &letter : item) {
        args.insert(std::string(1, tolower(letter)));
      }
      // todo
      item.erase(0);
    }
  }
  for (auto itr = input.begin(); itr != input.end(); ++itr) {
    if (itr->empty()) {
      input.erase(itr);
      --itr;
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
void OutputArgs(std::set<std::string> args) {
  using namespace std;
  cout << "args: " << endl;
  for (const auto &arg : args)
    cout << arg << endl;
}
void StripDotfiles(std::list<std::string> &names) {
  auto nameitr = names.begin();
  // iterate past . and ..
  nameitr++;
  nameitr++;

  if ((*nameitr)[0] == '.') {
    names.erase(nameitr);
    --nameitr;
  }
}
