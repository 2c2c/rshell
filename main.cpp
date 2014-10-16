#include <iostream>
#include <unistd.h>
#include <string>
#include <memory>
std::string UserHostInfo();
bool ContainsHome(const std::string& pwd, const std::string& loginname);
int main() {
    std::cout << UserHostInfo();
    
}

//uses unix calls to get username and hostname
//returns 'shellish' formatted str ie U@H: 
std::string UserHostInfo() {
    std::string loginname(getlogin());

    char* rawhost= new char[100];
    gethostname(rawhost,100);
    std::string hostname(rawhost);
    delete [] rawhost;

    std::string cwd(get_current_dir_name());

    if(ContainsHome)
        //swap /home/username with ~
    return loginname+"@"+hostname+":"+cwd+"$ ";
}

bool ContainsHome(const std::string& pwd, const std::string& loginname) {
    //search pwd
    //"/home/"+loginname

}
