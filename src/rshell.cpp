#include <algorithm>
#include <boost/tokenizer.hpp>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

const std::multimap<std::string, int>
DEFINED_OPS = {
    std::make_pair("&",2),
    std::make_pair("|",2),
    std::make_pair(";",1)
};

const std::vector<std::string> IMPLEMENTED_OPS{"&&","||",";"};

// handles what to do with finalized input state
void Execute(std::list<std::string>& input);

// assumes an operator in the first position of the input list
// extracts and pops it for determination of shell command flow
bool UseOperator(std::list<std::string>& input, bool prevcommandstate);

// used when an UseOperator returns false ie asdf && ls
// the first command is false and && must force a properly inputted second command
// to not execute. the command is extracted from the input list and nothing
// is done with it.
void DumpCommand(std::list<std::string>& input);

// assumes non operator in first position of the list
// tokens are extracted from the list until an operator or the end of
// the list is reached. the command is forked and execvp'd
bool UseCommand(std::list<std::string>& input);

// helper function that searched through global const IMPLEMENTED_OPS
// checks inputted string to see if it matches any within the vector
bool ContainsImplementedOp(std::string token);


// Returns true on >1 operators occuring back to back
// e.g. "&&&&&", ";;", etc.
bool FoundRepeat(const std::list<std::string>& input); 

// assumes all like-operators have been merged together
// this finds 'strings' of operators that are too long
// rebuilds a std pair from the global const map DEFINED_OPS
// uses it as param to string::find on each element of the input list
// if its found that means it CONTAINS the operator. checking if it
// doesn't equal at this point means it's using operator symbols of an
// incorrect type.
bool InvalidRepeatOp(const std::list<std::string>& input, std::pair<std::string,int> op); 

// helper function that calls RebuildOps with each element of global const
// DEFINED_OPS as 2nd parameter (the single instances of & | ;)
void CombineDefinedOps(std::list<std::string>& input); 

//unused check for unimplemented operators
bool UnimplementedOp(const std::list<std::string>& input, std::string op); 

//uses unix calls to get username and hostname
//returns 'shellish' formatted str ie usrname@host:~$ 
std::string UserHostInfo();

//output username@hostname$ and wait for command input
std::string Prompt();

//separates string by delimeters into vector elements
//if # is found it excludes it and anything beyond in the string
std::list<std::string> Split(const std::string& input);


//debugging inputlist outputter
void Output(std::list<std::string>& input);

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
        CombineDefinedOps(input);
        Execute(input);
    }
}
std::string UserHostInfo() {
    char* rawlogin = getlogin();
    if (rawlogin == NULL) {
        perror("getlogin returned NULL\n");
        exit(1);
    }
    std::string login(rawlogin);

    char *rawhost= new char[100];
    auto status = gethostname(rawhost,100);
    if(status == -1) {
        perror("gethostname failed\n");
    }

    std::string hostname(rawhost);
    delete [] rawhost;

    char* rawpwd = get_current_dir_name();
    if(rawpwd == NULL) {
        perror("get_current_dir_name returned NULL\n");
        exit(1);
    }
    std::string pwd(rawpwd);
    

    //handles /home/username -> ~/ shortcut
    std::string target = "/home/"+login+"/";
    if (pwd.find(target) == 0) {
        pwd.erase(0,target.size());
        pwd = "~/"+pwd;
    }
    return login+"@"+hostname+":"+pwd+"$ ";
}
std::string Prompt() {
    std::cout << "->" << UserHostInfo();
    std::string input;
    std::getline(std::cin, input);
    return input;
}
std::list<std::string> Split(const std::string& input) {
    using namespace boost;
    using namespace std;
    list<string> input_split;
    char_separator<char> sep(" ", "#&|;");   
    typedef tokenizer<char_separator<char>> tokener;

    tokener tokens(input, sep);

    //complexity increases if I handle # as bash does.
    for (const auto& t : tokens) {
        if (t == "#") break;
        input_split.push_back(t);
    }
    return input_split;
}
void Output(std::list<std::string>& input) {
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
        if(!tempstr.empty())
            front = input.insert(element, tempstr);
        front++;
    }
}
void CombineDefinedOps(std::list<std::string>& input) {
    for(const auto& op : DEFINED_OPS) {
        RebuildOps(input, op.first);
    }
}
bool FoundRepeat(const std::list<std::string>& input) {
    using namespace std;
    for (const auto& op : DEFINED_OPS) {
        if(InvalidRepeatOp(input,op)) {
            cout << "Invalid '" << op.first << "' usage found" << endl
                 << "known operator used an invalid amount of consecutive" << endl
                 << "times: e.g. '&&&' -> '&&' ?" << endl;
                 return true;
        }
    }
    return false;
}
bool InvalidRepeatOp(const std::list<std::string>& input, std::pair<std::string,int> pair) {
    std::string rebuilt_op = "";
    auto op_size = pair.second;
    while(op_size--) {
        rebuilt_op+= pair.first;
    }
    auto front = input.begin();
    auto back = input.end();
    while(front != back) {
        auto itr = std::find_if(front, back,
                [&](std::string elem) {
                if (elem.find(rebuilt_op) == std::string::npos)
                    return false;
                else
                    return true;
                });
        if (itr == back)
            return false;

        else if (*itr != rebuilt_op)
            break;
        else {
            front = itr;
            front++;
        }
    }
    return true;
}
bool UnimplementedOp(const std::list<std::string>& input, std::string op) {
    using namespace std;
    auto itr = find(input.begin(), input.end(), op);
    if(itr == input.end()) {
        cout << "operator '" << op << "' is unimplemented" << endl;
        return false;
    }
    return true;
}
bool UseCommand(std::list<std::string> &input) {
    using namespace std;

    //Take list of strings, make copies of their c_strs, and put into vector
    //a vector of char* can be used as char** if used as such
    vector<char *> vectorcommand;
    while (!input.empty() && !ContainsImplementedOp(input.front())) {
        string transferstr = input.front();
        input.pop_front();
        char* cstrcopy = new char[transferstr.size()+1];
        memcpy(cstrcopy, transferstr.c_str(), transferstr.size()+1);
        cstrcopy[transferstr.size()] = 0;
        vectorcommand.push_back(cstrcopy);
    }
    vectorcommand.push_back(NULL);

    char** rawcommand = &vectorcommand[0];
    pid_t wait_val;
    auto pid = fork();
    if (pid==-1) {
        perror("Error on fork\n");
        exit(1);
    }
    if (pid==0) {
        execvp(rawcommand[0], rawcommand);
        if(errno!=0) {
            perror("Error in execvp. Likely a nonexisting command?\n");
            exit(1);
        }
        for (size_t i = 0; i < vectorcommand.size(); i++)
            delete[] rawcommand[i];
    }
    else {
       wait_val = wait(0);
    }
    if (wait_val == -1) {
        perror("Error on waiting for child process to finish\n");
        exit(1);
    }
    return true;
}
bool ContainsImplementedOp(std::string token) {
    auto match = find(IMPLEMENTED_OPS.begin(), IMPLEMENTED_OPS.end(), token);
    if(match != IMPLEMENTED_OPS.end())
        return true;
    return false;
}
bool UseOperator(std::list<std::string>& input, bool prevcommandstate) {
    using namespace std;
    if(input.empty())
        return false;
    string op = input.front();
    input.pop_front();
    if (prevcommandstate == true) {
        if(op == ";")
            return true;
        else if(op == "&&")
            return true;
        else if(op == "||")
            return false;
    }
    else {
        if(op == ";")
            return true;
        else if(op == "&&")
            return false;
        else if(op == "||")
            return true;
    }
    //proper input ensures we never get down here, so im killing warning message
    //fixing this 'properly' would make it annoying to add more operators later
    return false;
}
void Execute(std::list<std::string>& input) {
    if(input.empty() || FoundRepeat(input))
        return;
    bool cmdstate = true;
    while(!input.empty()) {
        if(input.front() == "exit")
            exit(0);
        if (cmdstate)
            cmdstate = UseCommand(input);
        else 
            DumpCommand(input);
        cmdstate = UseOperator(input, cmdstate);
    }
}
void DumpCommand(std::list<std::string>& input) {
    while (!input.empty() && !ContainsImplementedOp(input.front())) {
        input.pop_front();
    }
}
