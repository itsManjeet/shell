#include <iostream>
#include <string>
#include <releax.hh>
#include <vector>
#include <sstream>
#include <map>
#include <memory>

#include <pwd.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>


enum GROUND {
    BACK = 48,
    FORE = 38,
};

using namespace std;

class Shell {

    int status;
    int commandStatus = 0;
    int lastcolor = 32;

    string readCMD(string);
    
    string getCWD();
    string genColorCode(int,GROUND);
    string genBlock(int color, string line, int col = 1);
    string resetColor() {
        return "\u001b[0m";
    }

public:
    void loop();

    int changeDir(vector<string>);

    vector<string> getInput();
    string genPrompt();
    int execute(vector<string>);

    void promptError(string);
    
};

string
Shell::getCWD()
{
    string path = get_current_dir_name();
    stringstream ss(path);
    string dir;
    while(getline(ss, dir, '/')) {
        path = dir;
    }
    return path;
}

string
Shell::readCMD(string cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

string
Shell::genBlock(int color, string line, int col)
{
    string block;
    if (col == 0) {
        block = genColorCode(color,BACK) + " " + line + " " + resetColor();
    }
    else if (col == 2) {
        block = genBlock(color, line) + genColorCode(color, FORE) + "" + resetColor();
    }
    else {
        block = genColorCode(lastcolor,FORE) + genColorCode(color, BACK) + " " + resetColor() + genColorCode(color, BACK) + line +  " " + resetColor();
    }
    lastcolor = color;
    return block;
}

string
Shell::genColorCode(int colorCode, GROUND ground)
{
    return "\u001b[" + to_string(ground) + ";5;" + to_string(colorCode) + "m";
}

string
Shell::genPrompt()
{

    struct passwd *pw;
    uid_t uid;

    uid = geteuid();
    pw = getpwuid(uid);
    string username;
    if (pw) {
        username = pw->pw_name;
    } else {
        username = "no name";
    }

    string block = this->genBlock(32, username, 0);
    if (filesys::exist("./.git")) {
        string output = this->readCMD("/usr/libexec/git-core/git-show-branch");
        stringstream ss(output);
        string branch;
        ss >> branch;
        branch = branch.substr(1, branch.length() - 2);

        block += this->genBlock(25, " " + branch);
    }

    if (commandStatus != 0) {
        block += this->genBlock(31, "! " + to_string(commandStatus));
    }
    block += this->genBlock(240, this->getCWD(), 2);

    return block + " ";
}

vector<string>
Shell::getInput()
{
    vector<string> args;
    string line, l;
    getline(cin, line);
    stringstream ss(line);
    while(ss >> l) {
        args.push_back(l);
    }

    return args;
}

void
Shell::loop()
{
    do {
        cout << this->genPrompt();  
        auto args = this->getInput();

        if (args.at(0) == "cd") {
            status = changeDir(args);
        } else if (args.at(0) == "exit") {
            status = 0;
        } else {
            status = this->execute(args);
        }

    } while(status);
}


int
Shell::changeDir(vector<string> args)
{
    if (args.size() >= 1) {
        promptError("expected dir name");
    } else {
        if (chdir(args.at(1).c_str()) != 0) {
            perror("shell");
        }
    }
    return 1;
}

void
Shell::promptError(string err)
{
    cout << "shell: "<< err << endl;
}

int
Shell::execute(vector<string> args)
{
    pid_t pid, wid;

    vector<char*> cargs{};

    for(auto &s: args) {
        cargs.push_back(&s.front());
    }
    cargs.push_back(NULL);

    pid = fork();
    if (pid == 0){
        if (execvp(cargs.data()[0], cargs.data()) != -1) {
            perror("shell");
        }
        exit(1);
    } else if (pid < 0) {
        perror("shell");
    } else {
        do {
            wid = waitpid(pid, &commandStatus, WUNTRACED);
        } while(!WIFEXITED(commandStatus) && !WIFSIGNALED(commandStatus));
    }

    return 1;
}

int
main(int ac, char** av)
{
    Shell shell;
    shell.loop();

    return 0;
}