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

void NormalList(std::map<std::string, int, std::locale> files, std::string dir);
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
    } else
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
      NormalList(names,file);
    }
  } else {
    // recursive
    if (!multifile)
      cout << endl << file << ": " << endl;
    if (longlist) {
      LongListBundle(names, file);
    } else {
      NormalList(names,file);
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
        continue;
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

  // foreground color begins as green, if its a directory switch it to blue
  // if we later find out the file isn't an executable strip the color
  // the strange order is due to directories having the same file priviledges
  // as executables
  string foreground_color = "\x1b[32;1";
  string end_color = "\x1b[0m";

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
  else if (S_ISDIR(buf.st_mode)) {
    filetype = "d";
    foreground_color = "\x1b[34;1";
  } else if (S_ISLNK(buf.st_mode)) {
    filetype = "l";
    foreground_color = "\x1b[36;1";
  } else if (S_ISSOCK(buf.st_mode))
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

  // strip the color if the file isn't an executable
  if (!(permissions[2] == 'x' || permissions[5] == 'x' ||
        permissions[8] == 'x')) {
    foreground_color = "\x1b[0;1";
  }

  // background color is concatenated to foreground color eventually
  // if there is no bg color fix the formatting for fg color
  // else add bg color and close the color tag
  string background_color = (file[0] == '.') ? ";47m" : "m";

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


  string merged_string = filetype + permissions + " " + links + " " + username +
                         " " + groupname + " " + filesize + " " + thetime +
                         " " + foreground_color + background_color + file +
                         end_color + "\n";
  cout << merged_string;
  return merged_string;
}

void LongListBundle(std::map<std::string, int, std::locale> files,
                    std::string dir) {
  using namespace std;
  // append / to directory if it doesn't already have. makes concatenation
  // simpler down the road
  if (dir.back() != '/')
    dir.push_back('/');
  // return early if empty
  if (files.empty()) {
    cout << "total 0" << endl;
    return;
  }

  long largest_filesize = 0;
  long block_total = 0;
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
    block_total += sizecheck.st_blocks;
  }
  block_total /= 2;
  cout << "total " << block_total << endl;
  string filesize_width = to_string(largest_filesize);
  for (const auto &file : files) {
    string information = LongList(dir + file.first, filesize_width.size());
  }
}
void NormalList(std::map<std::string, int, std::locale> files,
                std::string dir) {
  using namespace std;
  // append / to directory if it doesn't already have. makes concatenation
  // simpler down the road
  if (dir.back() != '/')
    dir.push_back('/');

  //quick and dirty color implementation
  size_t colorbuffer;
  vector<string> colorfiles;
  for (const auto& pair : files) {
    string full_path = dir + pair.first;
    struct stat buf;
    lstat(full_path.c_str(), &buf);
    if (errno != 0) {
      perror("longlist lstat error");
      exit(1);
    }
    string foreground_color = "\x1b[32;1";
    string end_color = "\x1b[0m";
    // default green
    if (S_ISLNK(buf.st_mode)) {
      foreground_color = "\x1b[36;1";
    } else if (S_ISDIR(buf.st_mode)) {
      // blue
      foreground_color = "\x1b[34;1";
    }

    if (!(buf.st_mode & S_IXUSR || buf.st_mode & S_IXGRP ||
          buf.st_mode & S_IXOTH)) {
      foreground_color = "\x1b[39;1";
    }

    string background_color = (pair.first[0] == '.') ? ";47m" : ";49m";

    string colored_filename =
        foreground_color + background_color + pair.first + end_color;

    colorfiles.push_back(colored_filename);
    colorbuffer = foreground_color.size() + background_color.size() + end_color.size();
  }

  // fixed columnsize
  // return early if empty
  if (files.empty()) {
    return;
  }
  // there are insecure ways of acquiring current terminal's column size
  // that I am avoiding purposely
  const size_t COLSIZE = 80;
  size_t widest_file = 0;
  for (const auto &pair : files) {
    if (pair.first.size() > widest_file)
      widest_file = pair.first.size();
  }
  // adding 2 accounts for the guaranteed 2 spaces gap
  widest_file += 2;
  size_t file_columns = COLSIZE / (widest_file);
  size_t count = 0;
  if (file_columns == 0) {
    file_columns = 1;
  }
  // multirow case
  if (file_columns < files.size()) {
    for (const auto &file : colorfiles) {
      count++;
      if (count == file_columns) {
        cout << left << file << endl;
        count = 0;
        continue;
      }
      cout << left << setw(widest_file+colorbuffer) << file;
    }
    cout << left << endl;
  }
  // single row case
  else {
    for (const auto &file : colorfiles) {
      cout << file<< "  ";
    }
    cout << endl;
  }
}
