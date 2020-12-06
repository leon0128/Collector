#ifndef SYSTEM_HPP
#define SYSTEM_HPP

#include <utility>
#include <string>
#include <cstdlib>

namespace SYSTEM
{

constexpr const char *command() noexcept
    {return "";}
template<class Str, class... Args>
std::string command(Str &&str, Args&&... args)
{
    std::string ret(str);
    ret.push_back(' ');
    ret += command(std::forward<Args>(args)...);
    return ret;
}

int system(const std::string &cmd)
    {return std::system(cmd.c_str());}
template<class... Args>
int system(Args&&... cmds)
    {return std::system(command(std::forward<Args>(cmds)...).c_str());}

}

#endif