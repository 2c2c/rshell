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
std::string LongList(std::string file);
void Print (std::string file);
using namespace std;
int main() {
  LongList(".");
}

void Print (std::string file) {
  std::string dirName = file;
  map<string,int> names;
  DIR *dirp = opendir(dirName.c_str());
  dirent *direntp;
  while ((direntp = readdir(dirp))) {
    names.emplace(make_pair(dirName+direntp->d_name, direntp->d_type));
    std::cout << direntp->d_name
              << std::endl; // use stat here to find attributes of file
  }
  closedir(dirp);
  for ( const auto& pair : names) {
    if (pair.second == DT_DIR) {
      cout << "NAME: " << pair.first << " " << pair.second << endl;
      DIR *dirp2 = opendir(pair.first.c_str());
      dirent *dirent2;
      cout << "---------" << endl;
      while ((dirent2 = readdir(dirp2))) {
        cout << dirent2->d_name << endl;
      }
      cout << "---------" << endl;
    }
  }
}

//  DT_BLK      This is a block device.
//  DT_CHR      This is a character device.
//  DT_DIR      This is a directory.
//  DT_FIFO     This is a named pipe (FIFO).
//  DT_LNK      This is a symbolic link.
//  DT_REG      This is a regular file.
//  DT_SOCK     This is a UNIX domain socket.
//  DT_UNKNOWN  The file type is unknown.
std::string LongList(std::string file) {
  using namespace std;
  struct stat *buf = new struct stat;
  stat(file.c_str(), buf);
  
  //filetype
  string filetype;
  if (buf->st_mode == S_IFBLK)
    filetype = "b";
  else if (buf->st_mode == S_IFCHR)
    filetype = "c";
  else if (buf->st_mode == S_IFIFO)
    filetype = "f";
  else if (buf->st_mode == S_IFREG)
    filetype = "-";
  else if (buf->st_mode == S_IFDIR)
    filetype = "d";
  else if (buf->st_mode == S_IFLNK)
    filetype = "l";
  else if (buf->st_mode == S_IFSOCK)
    filetype = "s";


    //permissions
    string permissions = "---------";
if (buf->st_mode & S_IRUSR) permissions[0] = 'r';//owner has read permission
if (buf->st_mode & S_IWUSR) permissions[1] = 'w';//owner has write permission
if (buf->st_mode & S_IXUSR) permissions[2] = 'x';//owner has execute permission
if (buf->st_mode & S_IRGRP) permissions[3] = 'r';//group has read permission
if (buf->st_mode & S_IWGRP) permissions[4] = 'w';//group has write permission
if (buf->st_mode & S_IXGRP) permissions[5] = 'x';//group has execute permission
if (buf->st_mode & S_IROTH) permissions[6] = 'r';//others have read permission
if (buf->st_mode & S_IWOTH) permissions[7] = 'w';//others have write permission
if (buf->st_mode & S_IXOTH) permissions[8] = 'x';//others have execute permission

  //num file links
  string links = to_string(buf->st_nlink);
  
  //username and groupname
  struct passwd *user;
  user = getpwuid(buf->st_uid);
  struct group *grp;
  grp = getgrgid(buf->st_gid);
  
  string username(user->pw_name);
  string groupname(grp->gr_name);

  //filesize
  string filesize = to_string(buf->st_size);


  //modification date vs creation date depending on which is newer
  auto date = buf->st_mtime > buf->st_ctime ? buf->st_mtime : buf->st_ctime;
  char *rawtime = ctime(&date);
  string time(rawtime);
  //remove day of week + space e.g. 'Sat '
  time.erase(0,4);
  //remove the newline
  time.pop_back();

  string merged_string = filetype + permissions + " " + links + " " + username +
                         " " + groupname + " " + filesize + " " + time + " " +
                         file;
                         cout<<merged_string;
  return merged_string;
}
