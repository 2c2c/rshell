#include <iostream>
#include <unistd.h>
#include <string>
#include <memory>
#include <vector>
#include <sstream>
#include <boost/tokenizer.hpp>
std::string UserHostInfo();
void Prompt();
bool ContainsHome(const std::string& pwd, const std::string& loginname);


int main() {

    std::cout << UserHostInfo();
    //execvp(list.c_str(), args);
    
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
    if(ContainsHome(cwd, loginname))
        std::cout << "hey";
    return loginname+"@"+hostname+":"+cwd+"$ ";
}

bool ContainsHome(const std::string& pwd, const std::string& loginname) {
    std::string target = "/home/"+loginname+"/";
    if (pwd.find(target) == 0) {
                return true;
    }
    return false;
}

void Prompt() {
    std::cout << UserHostInfo();
    std::string input;
    std::cin >> input;

}
