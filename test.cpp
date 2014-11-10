#include <ctime>
#include <iomanip>
#include <chrono>
#include <grp.h>
#include <pwd.h>
#include <map>
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
#include <vector>
std::string LongList(std::string file, size_t padding);
void LongListBundle(std::map<std::string,int, std::locale> files);
void Print(std::string file);
using namespace std;
int main() {
  Print(".");  
}
void Print(std::string file) {
  std::string dirName = file;
  //append / to directory if none exists already
  if (dirName.back() != '/')
    dirName.push_back('/');
  map<string, int, std::locale> names(std::locale("en_US.UTF-8"));
  DIR *dirp = opendir(dirName.c_str());
  dirent *direntp;
  while ((direntp = readdir(dirp))) {
    names.emplace(make_pair(direntp->d_name, direntp->d_type));
    std::cout << direntp->d_name
              << std::endl; // use stat here to find attributes of file
  }
  closedir(dirp);




  LongListBundle(names);
  //For (const auto &pair : names) {
  //  if (pair.second == DT_DIR) {
  //    cout << "NAME: " << pair.first << " " << pair.second << endl;
  //    DIR *dirp2 = opendir(pair.first.c_str());
  //    dirent *dirent2;
  //    cout << "---------" << endl;
  //    while ((dirent2 = readdir(dirp2))) {
  //      cout << dirent2->d_name << endl;
  //    }
  //    cout << "---------" << endl;
  //  }
  //}
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
