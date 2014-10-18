#include <iostream>
#include <unistd.h>
#include <string>
#include <memory>
#include <vector>
#include <sstream>
#include <algorithm>
#include <boost/tokenizer.hpp>

//uses unix calls to get username and hostname
//returns 'shellish' formatted str ie U@H: 
std::string UserHostInfo();

//output username@hostname$ and wait for command input
std::string Prompt();

std::vector<std::string> Split(const std::string& input);
void Parse(std::vector<std::string>& input);
int main() {
    auto cmd = Prompt();
    Split(cmd);
    //execvp(list.c_str(), args);
    
}

std::string UserHostInfo() {
    std::string loginname(getlogin());

    char *rawhost= new char[100];
    gethostname(rawhost,100);
    std::string hostname(rawhost);
    delete [] rawhost;

    std::string pwd(get_current_dir_name());
    
    //handles /home/username -> ~/ shortcut
    std::string target = "/home/"+loginname+"/";
    if (pwd.find(target) == 0) {
        pwd.erase(0,target.size());
        pwd = "~/"+pwd;
    }

    return loginname+"@"+hostname+":"+pwd+"$ ";
}

std::string Prompt() {
    std::cout << UserHostInfo();
    std::string input;
    std::getline(std::cin, input);
    return input;
}

//&& || ;
//#

std::vector<std::string> Split(const std::string& input) {
    using namespace boost;
    using namespace std;
    vector<string> input_split;
    char_separator<char> sep("", " #&|;");   
    typedef tokenizer<char_separator<char>> tokener;

    tokener tokens(input, sep);

    //complexity increases if I handle # as bash does.
    for (const auto& t : tokens) {
        if (t == "#") break;
        input_split.push_back(t);
    }

    return input_split;
}

void Parse(std::vector<std::string>& input) {


}
