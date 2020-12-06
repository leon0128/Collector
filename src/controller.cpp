#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

#include "git.hpp"
#include "path.hpp"
#include "configure.hpp"
#include "controller.hpp"

Controller::Controller()
    :mRepositories()
{
}

Controller::~Controller()
{
    for(auto &&p : mRepositories)
        delete p.second;
}

bool Controller::initialize()
{
    if(!Configure::initialize())
    {
        std::cerr << "init error:\n"
            "    what: failed to call Configure::initialize().\n"
            << std::flush;
        return false;
    }
    
    if(!loadFromDirectory())
    {
        std::cerr << "init error:\n"
            "    what: failed to call Controller::loadFromDirectory().\n"
            << std::flush;
        return false;
    }

    return true;
}

void Controller::run()
{
    while(process())
        std::this_thread::sleep_for(std::chrono::hours(Configure::loopRange()));
}

bool Controller::process()
{
    if(!loadFromJson())
    {
        std::cerr << "load-repositories error:\n"
            << "what: failed to load repositories from json file.\n"
            << std::flush;
        return false;
    }

    if(!clone())
    {
        std::cerr << "clone-repositories error:\n"
            "    what: failed to clone repositories.\n"
            << std::flush;
        return false;
    }

    if(!pull())
    {
        std::cerr << "pull-branch error:\n"
             "    what: failed to pull remote branch.\n"
            << std::flush;
        return false;
    }

    if(!log())
    {
        std::cerr << "get-log error:\n"
            "    what: failed to get log.\n"
            << std::flush;
        return false;
    }

    if(!diff())
    {
        std::cerr << "get-diff error:\n"
            "    what: failed to get difference.\n"
            << std::flush;
        return false;
    }

    return true;
}

bool Controller::loadFromJson()
{
    if(!Configure::reloadRepositories())
    {
        std::cerr << "loadFromJson warning:\n"
            "    what: failed to load json file.\n"
            "    approach: use previous value.\n"
            << std::flush;
        return true;
    }

    std::unordered_map<std::string, GIT::Repository*> newReps;
    for(auto &&p : Configure::repositoriesMap())
    {
        auto iter = mRepositories.find(p.first);
        if(iter == mRepositories.end())
            newReps.emplace(p.first
                , new GIT::Repository(Configure::repositoriesDir() / p.first
                    , p.second));
        else
        {
            if(iter->second->url() == p.second)
                newReps.emplace(p.first, iter->second);
            else
            {
                iter->second->remove(Configure::differenceDir() / p.first);
                delete iter->second;
                
                newReps.emplace(p.first
                    , new GIT::Repository(Configure::repositoriesDir() / p.first
                        , p.second));

                std::cerr << "load-repositories warning:\n"
                    "    what: change url.\n"
                    "    name: " << p.first << "\n"
                    "    old url: " << iter->second->url() << "\n"
                    "    new url: " << p.second
                    << std::endl;
            }

            mRepositories.erase(iter);
        }
    }

    mRepositories.merge(newReps);

    return true;
}

bool Controller::loadFromDirectory()
{
    if(!PATH::isValid(Configure::repositoriesDir(), std::filesystem::file_type::directory))
        return false;

    for(auto &&de : std::filesystem::directory_iterator(Configure::repositoriesDir()))
    {
        GIT::Repository rep(de.path(), std::string());
        if(!rep.setUrl())
        {
            std::filesystem::remove_all(de.path());

            std::cerr << "load-repositories warning:\n"
                "    what: failed to get url from directory.\n"
                "    path: " << rep.path().string() << "\n"
                "    approach: remove this file or directory.\n"
                << std::flush;
        }
        else
            mRepositories.emplace(de.path().filename(), new GIT::Repository(std::move(rep)));
    }

    return true;
}

bool Controller::clone()
{
    std::vector<std::string> rmvec;
    for(auto &&p : mRepositories)
    {
        if(!p.second->clone())
        {
            rmvec.push_back(p.first);
            p.second->remove(Configure::differenceDir() / p.first);
            delete p.second;

            std::cerr << "clone warning:\n"
                "    what: failed to clone repository.\n"
                "    name: " << p.first << "\n"
                "    approach: remove this repository.\n"
                << std::flush;
        }
    }

    for(auto &&s : rmvec)
        mRepositories.erase(mRepositories.find(s));

    return true;
}

bool Controller::pull()
{
    std::vector<std::string> rmvec;
    for(auto &&p : mRepositories)
    {
        if(!p.second->pull())
        {
            rmvec.push_back(p.first);
            p.second->remove(Configure::differenceDir() / p.first);
            delete p.second;

            std::cerr << "pull warning:\n"
                "    what: failed to pull remote repository.\n"
                "    name: " << p.first << "\n"
                "    approach: remove this repository.\n"
                << std::flush;
        }
    }

    for(auto &&s : rmvec)
        mRepositories.erase(mRepositories.find(s));

    return true;
}

bool Controller::log()
{
    std::vector<std::string> rmvec;
    for(auto &&p : mRepositories)
    {
        if(!p.second->log(std::filesystem::temp_directory_path() / p.first))
        {
            rmvec.push_back(p.first);
            p.second->remove(Configure::differenceDir() / p.first);
            delete p.second;

            std::cerr << "log warning:\n"
                "    what: failed to get log information.\n"
                "    name: " << p.first << "\n"
                "    approach: remove this repository.\n"
                << std::flush;
        }
    }

    for(auto &&s : rmvec)
        mRepositories.erase(mRepositories.find(s));
    
    return true;
}

bool Controller::diff()
{
    std::vector<std::string> rmvec;
    for(auto &&p : mRepositories)
    {
        if(!p.second->diff(std::filesystem::temp_directory_path() / p.first
            , Configure::differenceDir() / p.first))
        {
            rmvec.push_back(p.first);
            p.second->remove(Configure::differenceDir() / p.first);
            delete p.second;

            std::cerr << "diff warning:\n"
                "    what: failed to get difference information.\n"
                "    name: " << p.first << "\n"
                "    approach: remove this repository.\n"
                << std::flush;
        }
    }

    for(auto &&s : rmvec)
        mRepositories.erase(mRepositories.find(s));
    
    return true;
}