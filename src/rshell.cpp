#include <iostream>
#include <unistd.h>
#include <string>
#include <memory>
#include <vector>
#include <list>
#include <sstream>
#include <algorithm>
#include <boost/tokenizer.hpp>

const std::vector<std::string>
DEFINED_OPERATORS =
{"&&", "||", ";"};


void CombineDefinedOps(std::list<std::string>& input); 

//uses unix calls to get username and hostname
//returns 'shellish' formatted str ie usrname@host:~$ 
std::string UserHostInfo();

//output username@hostname$ and wait for command input
std::string Prompt();

//separates string by delimeters into vector elements
//if # is found it excludes it and anything beyond in the string
std::list<std::string> Split(const std::string& input);


void Parse(std::list<std::string>& input);

// takes the input list and an operator character and merges all repeating instances of that character within the list
// operators in shell can use the same symbol a different amount of times to represent different things
// ie bitwise & vs and &&
// the delimiter method I used to separate the arguments in the first place only had single character delimiting available
// since the function is general it will:
// avoid having to create new handcrafted parses when more features have to be added
// make bug checking general and simple (is & implemented? is &&& implemented? if not theyre bugs)
void RebuildOps(std::list<std::string>& input, std::string op);

int main() {
    while(true) {
        auto cmd = Prompt();
        auto input = Split(cmd);
        RebuildOps(input, "&");

        Parse(input);
    }

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

std::list<std::string> Split(const std::string& input) {
    using namespace boost;
    using namespace std;
    list<string> input_split;
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

void Parse(std::list<std::string>& input) {
    using namespace std;
    for (const auto& e : input)
        cout << e << endl;

}
void RebuildOps(std::list<std::string>& input, std::string op) {
    using namespace std;
    auto front = input.begin();
    auto back = input.end();
    while (front != back) {
        auto element = find(front, back, op);
        int count = 0;
        while (element != back) {
            if (*element == op) {
                element = input.erase(element);
                count++;
            }
            else {
                break;
            }
        }
        std::string tempstr = "";
        while (count--) {
            tempstr += op;
        }
        front = input.insert(element, tempstr);
        front++;
    }
}
void CombineDefinedOps(std::list<std::string>& input) {
    for(const auto& op : DEFINED_OPERATORS) {
        auto single = op.substr(0,1);
        RebuildOps(input, single);
    }
}
