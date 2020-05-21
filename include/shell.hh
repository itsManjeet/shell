#ifndef __SHELL__
#define __SHELL__

#include <iostream>
#include <string>
#include <releax>
#include <vector>
#include <sstream>
#include <map>
#include <memory>

#include <termios.h>
#include <pwd.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

enum GROUND {
    BACK = 48,
    FORE = 38,
};

class Shell {

    termios oldt;
    int status;
    int commandStatus = 0;
    int lastcolor = 32;

    std::string readCMD(std::string);
    
    std::string getCWD();
    std::string genColorCode(int,GROUND);
    std::string genBlock(int color, std::string line, int col = 1, bool light = false);
    std::string resetColor() {
        return "\u001b[0m";
    }

    void autoComplete(std::vector<std::string>);

public:
    // Shell();

    // virtual ~Shell();

    void loop();

    int changeDir(std::vector<std::string>);

    std::vector<std::string> getInput();
    std::string genPrompt();
    int execute(std::vector<std::string>);

    void promptError(std::string);
    
};


#endif