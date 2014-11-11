#include <stdlib.h>
#include <iomanip>
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

void NormalList(std::map<std::string, int, std::locale> files);
// print single line of a longlist output
std::string LongList(std::string file, size_t padding);
// manages a directory of longlist output
void LongListBundle(std::map<std::string, int, std::locale> files,
                    std::string dir);

// puts the argv input from the program into a list
std::list<std::string> GetInput(int argc, char **argcv);
void StripDotfiles(std::map<std::string, int, std::locale> &names);

void Print(std::string file, std::set<std::string> args, bool multifile);
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
  // empty input case, append . so it works as current directory
  if (input.empty())
    input.push_front(".");
  bool multifile;
  for (auto &filearg : input) {
    if (input.size() > 1) {
      cout << filearg << ": " << endl;
      multifile = true;
    }
    else
      multifile = false;
    Print(filearg, args, multifile);
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
void StripDotfiles(std::map<std::string, int, std::locale> &names) {
  // iterate past . and ..
  auto target = names.find(".");
  names.erase(target);
  target = names.find("..");
  names.erase(target);
  auto nameitr = names.begin();

  while (nameitr != names.end()) {
    if ((*nameitr).first[0] == '.') {
      names.erase(nameitr);
      nameitr = names.begin();
    }
    ++nameitr;
  }
}
void Print(std::string file, std::set<std::string> args, bool multifile) {
  using namespace std;
  map<string, int, std::locale> names(std::locale("en_US.UTF-8"));
  DIR *dirp = opendir(file.c_str());
  if (errno != 0) {
    perror("opendir error");
    exit(1);
  }
  dirent *direntp;
  while ((direntp = readdir(dirp))) {
    if (errno != 0) {
      perror("readdir error");
      exit(1);
    }
    names.emplace(make_pair(direntp->d_name, direntp->d_type));
  }
  closedir(dirp);
  if (errno != 0) {
    perror("closedir error");
    exit(1);
  }

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
    if (longlist == true) {
      LongListBundle(names, file);
    } else {
      NormalList(names);
    }
  } else {
    // recursive
    if (!multifile)
      cout << endl << file << ": " << endl;
    if (longlist) {
      LongListBundle(names, file);
    } else {
      NormalList(names);
    }
    // check empty filelist before compariing iterators
    if (names.empty())
      return;

    auto itr = names.begin();
    string append_dir;
    while (!names.empty() && itr != names.end()) {
    // if . / .. are in the list iterate over them
      if (itr->first == "." || itr->first == "..") {
        ++itr;
      }
      if (itr->second == DT_DIR) {
        // append / to directory if it doesn't already have. makes concatenation
        // simpler down the road
        if (file.back() != '/')
          file.push_back('/');
        append_dir = itr->first;
        names.erase(itr++);
        Print(file + append_dir, args, false);
      } else {
        ++itr;
      }
    }
  }
}
std::string LongList(std::string file, size_t padding) {
  using namespace std;
  struct stat buf;
  lstat(file.c_str(), &buf);
  if (errno != 0) {
    perror("longlist lstat error");
    exit(1);
  }

  // filetype
  string filetype;
  if (S_ISBLK(buf.st_mode))
    filetype = "b";
  else if (S_ISCHR(buf.st_mode))
    filetype = "c";
  else if (S_ISFIFO(buf.st_mode))
    filetype = "f";
  else if (S_ISREG(buf.st_mode))
    filetype = "-";
  else if (S_ISDIR(buf.st_mode))
    filetype = "d";
  else if (S_ISLNK(buf.st_mode))
    filetype = "l";
  else if (S_ISSOCK(buf.st_mode))
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
  if (errno != 0) {
    perror("pwuid error");
    exit(1);
  }
  struct group grp;
  grp = *getgrgid(buf.st_gid);
  if (errno != 0) {
    perror("groupid error");
    exit(1);
  }

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
  // remove the ':ss yyyy\n' piece of the date. this is 9 chars until the year
  // 10000
  thetime.erase(thetime.size() - 9);

  // erase prepended directories from filename
  auto last_slash = file.begin();
  for (auto itr = file.begin(); itr != file.end(); ++itr) {
    if (*itr == '/') {
      last_slash = itr;
    }
  }
  // the range is [first, last)
  last_slash++;
  file.erase(file.begin(), last_slash);

  string merged_string = filetype + permissions + " " + links + " " + username +
                         " " + groupname + " " + filesize + " " + thetime +
                         " " + file + "\n";
  cout << merged_string;
  return merged_string;
}

void LongListBundle(std::map<std::string, int, std::locale> files,
                    std::string dir) {
  // append / to directory if it doesn't already have. makes concatenation
  // simpler down the road
  if (dir.back() != '/')
    dir.push_back('/');
  using namespace std;
  // return early if empty
  if (files.empty()) {
    cout << "total 0" << endl;
    return;
  }

  long largest_filesize = 0;
  long block_total = 0;
  long size_total = 0;
  struct stat sizecheck;
  for (const auto &file : files) {
    std::string path = dir + file.first;
    lstat(path.c_str(), &sizecheck);
    if (errno != 0) {
      perror("longlist lstat error");
      exit(1);
    }
    if (sizecheck.st_size > largest_filesize) {
      largest_filesize = sizecheck.st_size;
    }
  cout << "blocksize" << sizecheck.st_blksize << endl;
  cout << "num blocks" << sizecheck.st_blocks << endl;
  size_total += sizecheck.st_size;
  block_total += sizecheck.st_blocks;
  }
  block_total/= 2;
  // Output total block size before doing individual longlist lines
  // TODO: fix this the numbers don't match!
  cout << "total " << block_total << endl;
  string filesize_width = to_string(largest_filesize);
  for (const auto &file : files) {
    string information = LongList(dir + file.first, filesize_width.size());
  }
}
// TODO columns based on filesize
// width assumed as 80 col
void NormalList(std::map<std::string, int, std::locale> files) {
  using namespace std;
  // fixed columnsize
  // return early if empty
  if (files.empty()) {
    return;
  }
  const size_t COLSIZE = 80;
  size_t widest_file = 0;
  for (const auto &pair : files) {
    if (pair.first.size() > widest_file)
      widest_file = pair.first.size();
  }
  size_t file_columns = COLSIZE / (widest_file + 2);
  size_t count = 0;

  // multirow case
  if (file_columns < files.size()) {
    for (const auto &pair : files) {
      cout << setw(widest_file) << left << pair.first << "  ";
      if (count == file_columns) {
        cout << endl;
        count = 0;
        continue;
      }
      count++;
    }
    cout << endl;
  }
  // single row case
  else {
    for (const auto &pair : files) {
      cout << pair.first << "  ";
    }
    cout << endl;
  }
}
