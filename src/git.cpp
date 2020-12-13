#include <utility>
#include <iostream>

#include <boost/property_tree/json_parser.hpp>
#include <boost/optional.hpp>

#include "system.hpp"
#include "path.hpp"
#include "git.hpp"

namespace GIT
{

bool Repository::clone() const
{
    if(PATH::isExist(path() / ".git", std::filesystem::file_type::directory))
        return true;

    std::string cmd = SYSTEM::command("git"
        , "clone"
        , "--quiet"
        , url()
        , path().string()
        , ">"
        , "/dev/null"
        , "2>&1");
    if(SYSTEM::system(cmd) == 0)
        return true;
    else
        return outSystemError(cmd);
}

bool Repository::pull() const
{
    std::string cmd(SYSTEM::command("git"
        , "-C"
        , path().string()
        , "pull"
        , "--quiet"
        , "--all"
        , ">"
        , "/dev/null"
        , "2>&1"));
    if(SYSTEM::system(cmd) == 0)
        return true;
    else
        return outSystemError(cmd);
}

bool Repository::log(const std::filesystem::path &logpath) const
{
    if(!PATH::isValid(logpath))
        return outFileError(logpath);

    std::string cmd(SYSTEM::command("git"
        , "-C"
        , path().string()
        , "log"
        , "--pretty=format:\"%H\n%s\""
        , "--output=" + logpath.string()
        , ">"
        , "/dev/null"
        , "2>&1"));
    if(SYSTEM::system(cmd) == 0)
        return true;
    else
        return outSystemError(cmd);
}

bool Repository::diff(const std::filesystem::path &input
    , const std::filesystem::path &output) const
{
    if(!PATH::isExist(input))
        return outFileError(input);

    if(!PATH::isValid(output, std::filesystem::file_type::directory))
        return outFileError(output);

    std::string str = PATH::read(input);

    for(std::string::size_type pos = 0; pos < str.size();)
    {
        std::string hash, subject;
        pos = PATH::getLine(str, hash, pos);
        pos = PATH::getLine(str, subject, pos);

        std::filesystem::path json(output / (hash + ".json"));
        if(!PATH::isExist(json))
        {
            if(!outputDiff(json, hash, subject))
            {
                std::cerr << "git-diff warning:\n"
                    "    what: failed to output difference file.\n"
                    "    path: " << path().string() << "\n"
                    "    url: " << url() << "\n"
                    "    hash: " << hash << "\n"
                    "    approach: ignore this hash.\n"
                    << std::flush;
            }
        }
    }

    return true;
}

bool Repository::show(const std::filesystem::path &output
    , const std::string &hash) const
{
    if(!PATH::isValid(output))
        return outFileError(output);

    std::string cmd(SYSTEM::command("git"
        , "-C"
        , path().string()
        , "show"
        , "--oneline"
        , "--patch"
        , "--unified=0"
        , "--minimal"
        , "--no-color"
        , "--src-prefix=\"\""
        , "--dst-prefix=\"\""
        , "--output-indicator-new=\"+\""
        , "--output-indicator-old=\"-\""
        , "--ignore-blank-lines"
        , "--ignore-space-change"
        , "--output=" + output.string()
        , hash
        , ">"
        , "/dev/null"
        , "2>&1"));

    if(SYSTEM::system(cmd) == 0)
        return true;
    else
        return outSystemError(cmd);
}

void Repository::remove(const std::filesystem::path &diffdir) const
{
    if(PATH::isExist(path(), std::filesystem::file_type::directory))
        std::filesystem::remove_all(path());
    if(PATH::isExist(diffdir, std::filesystem::file_type::directory))
        std::filesystem::remove_all(diffdir);
}

bool Repository::setUrl()
{
    if(!PATH::isExist(path(), std::filesystem::file_type::directory))
        return false;
    
    std::filesystem::path tmp(std::filesystem::temp_directory_path() / path().filename());

    std::string cmd = SYSTEM::command("git"
        , "-C"
        , path().string()
        , "config"
        , "--get"
        , "remote.origin.url"
        , ">"
        , tmp.string());
    if(SYSTEM::system(cmd) == 0)
    {
        std::string file(PATH::read(tmp));
        PATH::getLine(file, mUrl);
        
        return true;
    }
    else
        return outSystemError(cmd);
}

bool Repository::parseShow(const std::filesystem::path &showpath
    , boost::property_tree::ptree &tree) const
{
    std::string str(PATH::read(showpath));

    std::string line;
    for(std::string::size_type fp = str.find("\n---"); fp != std::string::npos; fp = str.find("\n---", fp))
    {
        fp++;
        boost::property_tree::ptree filenode;

        fp = PATH::getLine(str, line, fp);
        filenode.put("src", line.substr(4));

        fp = PATH::getLine(str, line, fp);
        filenode.put("dst", line.substr(4));

        std::string::size_type nextfp = str.find("\n---", fp - 1);
        boost::property_tree::ptree hunknode;
        for(std::string::size_type hp = str.find("\n@@", fp - 1); hp != std::string::npos || hp > nextfp; hp = str.find("\n@@", hp - 1))
        {
            hp++;
            boost::property_tree::ptree hunktree;

            hp = PATH::getLine(str, line, hp);
            hunktree.put("info", line.substr(0, line.find("@@", 2) + 2));
            
            boost::property_tree::ptree subnode, addnode;
            while(hp < str.size() && str[hp] == '+' || str[hp] == '-')
            {
                hp = PATH::getLine(str, line, hp);
                boost::property_tree::ptree ele;
                ele.put("", line.substr(1));
                (line.front() == '+' ? addnode : subnode).push_back(std::make_pair("", ele));

                fp = hp - 1;
            }
            
            if(!subnode.empty())
                hunktree.add_child("sub", subnode);
            if(!addnode.empty())
                hunktree.add_child("add", addnode);

            hunknode.push_back(std::make_pair("", hunktree));
        }

        if(!hunknode.empty())
            filenode.add_child("hunk", hunknode);

        tree.push_back(std::make_pair("", filenode));
    }

    return true;
}

bool Repository::outputDiff(const std::filesystem::path &output
    , const std::string &hash
    , const std::string &subject) const
{
    using namespace boost::property_tree;
    using namespace boost;
    
    if(!PATH::isValid(output))
        return outFileError(output);

    if(!show(std::filesystem::temp_directory_path() / hash, hash))
        return false;
    
    boost::property_tree::ptree difftree;
    if(!parseShow(std::filesystem::temp_directory_path() / hash, difftree))
        return false;

    ptree tree;
    tree.put("hash", hash);
    tree.put("subject", subject);
    if(!difftree.empty())
        tree.add_child("difference", difftree);

    write_json(output.string(), tree);
    return true;
}

bool Repository::outSystemError(const std::string &cmd) const
{
    std::cerr << "system error:\n"
        "    what: failed to execute system command.\n"
        "    path: " << path().string() << "\n"
        "    url: " << url() << "\n"
        "    cmd: " << cmd
        << std::endl;
    return false;
}

bool Repository::outFileError(const std::filesystem::path &file) const
{
    std::cerr << "file error:\n"
        "    what: failed to operate file.\n"
        "    path: " << path().string() << "\n"
        "    url: " << url() << "\n"
        "    file: " << file.string()
        << std::endl;
    return false;
}

}