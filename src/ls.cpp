#include <pwd.h>
#include <grp.h>
#include <locale>
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

// print single line of a longlist output
std::string LongList(std::string file, size_t padding);
// manages a directory of longlist output
void LongListBundle(std::map<std::string,int, std::locale> files);
// puts the argv input from the program into a list
std::list<std::string> GetInput(int argc, char **argcv);
void StripDotfiles(std::map<std::string,int, std::locale> &names);

void Print(std::string file, std::set<std::string> args);
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
  if (UnknownArgs(input)) {
    cout << "Unknown/unimplemented arg sent (not l, a or R)" << endl;
    exit(1);
  }
  auto args = Split(input);
  //empty input case, append . so it works as current directory
  if (input.empty()) 
    input.push_front(".");
  Print(input.front(), args);

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
        args.insert(std::string(1, letter));
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
        if (letter != 'l' && letter != 'a' && letter != 'R')
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
void StripDotfiles(std::map<std::string,int, std::locale> &names) {
  auto nameitr = names.begin();
  // iterate past . and ..
  nameitr++;
  nameitr++;

  while (nameitr != names.end()) {
    if ((*nameitr).first[0] == '.') {
      names.erase(nameitr);
      --nameitr;
    }
    ++nameitr;
  }
}
void Print(std::string file, std::set<std::string> args) {
  using namespace std;
  //append / to directory if it doesn't already have. makes concatenation
  // simpler down the road
  if (file.back() != '/')
    file.push_back('/');
  map<string, int, std::locale> names(std::locale("en_US.UTF-8"));
  DIR *dirp = opendir(file.c_str());
  dirent *direntp;
  while ((direntp = readdir(dirp))) {
    names.emplace(make_pair(direntp->d_name, direntp->d_type));
  }
  closedir(dirp);

  auto argcheck = args.find("a");
  if (argcheck == std::end(args))
    StripDotfiles(names);

  bool longlist;
  argcheck = args.find("l");
  if (argcheck == std::end(args)) {
    longlist = false;
  } else {
    longlist = true;
  }
  argcheck = args.find("R");
  if (argcheck == std::end(args)) {
    if(longlist==true) {
      LongListBundle(names);
    }else{}//else normal print 
      
  } else {
    // recursive
  }
  std::cout << "excuse me" << std::endl;
}
std::string LongList(std::string file, size_t padding) {
  using namespace std;
  struct stat buf;
  lstat(file.c_str(), &buf);

  // filetype
  string filetype;
  if (S_ISBLK(buf.st_mode ))
    filetype = "b";
  else if (S_ISCHR(buf.st_mode ))
    filetype = "c";
  else if (S_ISFIFO(buf.st_mode ))
    filetype = "f";
  else if (S_ISREG(buf.st_mode ))
    filetype = "-";
  else if (S_ISDIR(buf.st_mode ))
    filetype = "d";
  else if (S_ISLNK(buf.st_mode ))
    filetype = "l";
  else if (S_ISSOCK(buf.st_mode ))
    filetype = "s";

  // permissions
  string permissions = "---------";
  if (buf.st_mode & S_IRUSR)
    permissions[0] = 'r'; // owner has read permission
  if (buf.st_mode & S_IWUSR)
    permissions[1] = 'w'; // owner has write permission
  if (buf.st_mode & S_IXUSR)
    permissions[2] = 'x'; // owner has execute permission
  if (buf.st_mode & S_IRGRP)
    permissions[3] = 'r'; // group has read permission
  if (buf.st_mode & S_IWGRP)
    permissions[4] = 'w'; // group has write permission
  if (buf.st_mode & S_IXGRP)
    permissions[5] = 'x'; // group has execute permission
  if (buf.st_mode & S_IROTH)
    permissions[6] = 'r'; // others have read permission
  if (buf.st_mode & S_IWOTH)
    permissions[7] = 'w'; // others have write permission
  if (buf.st_mode & S_IXOTH)
    permissions[8] = 'x'; // others have execute permission

  // num file links
  // giving 3 characters of space hardcoded for time purposes
  string links = to_string(buf.st_nlink);
  if (links.size() == 1)
    links.insert(links.begin(), 2, ' ');
  else if (links.size() == 2)
    links.insert(links.begin(), 1, ' ');

  // username and groupname
  struct passwd user;
  user = *getpwuid(buf.st_uid);
  struct group grp;
  grp = *getgrgid(buf.st_gid);

  string username(user.pw_name);
  string groupname(grp.gr_name);

  // filesize
  string filesize = to_string(buf.st_size);
  filesize.insert(filesize.begin(), padding - filesize.size(), ' ');

  // modification date vs creation date depending on which is newer
  auto date = (buf.st_mtime > buf.st_ctime) ? buf.st_mtime : buf.st_ctime;
  char *rawtime = ctime(&date);
  string thetime(rawtime);
  // remove day of week + space e.g. 'Sat '
  thetime.erase(0, 4);
  // remove the ':ss yyyy\n' piece of the date. this is 9 chars until the year 10000
  thetime.erase(thetime.size() - 9);

  string merged_string = filetype + permissions + " " + links + " " + username +
                         " " + groupname + " " + filesize + " " + thetime + " " +
                         file + "\n";
  cout << merged_string;
  return merged_string;
}

void LongListBundle(std::map<std::string,int, std::locale> files) {
  using namespace std;
  long largest_filesize = 0;
  long block_total = 0;
  for (const auto &file : files) {
    struct stat sizecheck;
    lstat(file.first.c_str(), &sizecheck);
    if (sizecheck.st_size > largest_filesize) {
      largest_filesize = sizecheck.st_size;
      //TODO
      if(file.first != "." || file.first != "..")
        block_total += sizecheck.st_blocks;
    }
  }
  //Output total block size before doing individual longlist lines
  //TODO: fix this the numbers don't match!
  cout << "total " << block_total << endl;
  string filesize_width = to_string(largest_filesize);
  for (const auto& file : files) {
    string information = LongList(file.first, filesize_width.size());
  }
}
