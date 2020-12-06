#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP

#include <unordered_map>
#include <string>

namespace GIT{class Repository;}

class Controller
{
private:

public:
    Controller();
    ~Controller();

    bool initialize();

    void run();

private:
    bool process();

    bool loadFromJson();
    bool loadFromDirectory();
    bool clone();
    bool pull();
    bool log();
    bool diff();

    std::unordered_map<std::string, GIT::Repository*> mRepositories;
};

#endif