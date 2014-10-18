#include <iostream>
#include <unistd.h>
#include <string>
#include <memory>
#include <vector>
#include <list>
#include <sstream>
#include <algorithm>
#include <boost/tokenizer.hpp>

//uses unix calls to get username and hostname
//returns 'shellish' formatted str ie U@H: 
std::string UserHostInfo();

//output username@hostname$ and wait for command input
std::string Prompt();

//separates string by delimeters into vector elements
//if # is found it excludes it and anything beyond in the string
std::list<std::string> Split(const std::string& input);


void Parse(std::list<std::string>& input);



//return false on single instances of an operator
//this simplifies && and || parsing since im not implementing
//bitwise operators
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
    while(front != back) {
        auto element = find(front, back, op);
        auto next = find(front, back, op);

        cout<< "front element" << endl;
        //escape the last possible case
        if (element == back)
            break; 

        while(element != back) {
            next++;
            if (next == back) {
                break;
            }
            else if (*element == *next) {
                cout<< "found a next" << endl;
                auto merged_symbols = *element + *next;
                element = input.erase(element);
                element = input.insert(element, merged_symbols);
                element = input.erase(next);
            }
            else {
                break;
            }
        }
        front = element;
        front++;
    }
}
