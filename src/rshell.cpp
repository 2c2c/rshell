#include <iostream>
#include <unistd.h>
#include <string>
#include <memory>
#include <vector>
#include <sstream>
#include <boost/tokenizer.hpp>
std::string UserHostInfo();
void Prompt();
void Parse(const char** argslist);
std::vector<std::string> ParseDashArg(const std::string& dashlist);
bool ContainsHome(const std::string& pwd, const std::string& loginname);


int main() {
    std::string list = "ls";

    const char** test = new const char*[10];
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
    if(ContainsHome)
        std::cout << "hey";
    return loginname+"@"+hostname+":"+cwd+"$ ";
}

bool ContainsHome(const std::string& pwd, const std::string& loginname) {
    std::string target = "/home/"+loginname+"/";
    if (pwd.find(target) == 0)
        return true;
    return false;
}

void ArgsFill(const char** argslist) {



}

void Prompt() {
    std::cout << UserHostInfo();
    std::string input;
    std::cin >> input;

}
std::vector<std::string> ParseDashArg(const std::string& dashlist) {



}
