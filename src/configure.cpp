#include <exception>
#include <iostream>

#include <boost/optional.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "path.hpp"
#include "configure.hpp"

bool Configure::initialize()
{
    return loadConfigure() && loadRepositories();
}

bool Configure::reloadRepositories()
{
    return loadRepositories();
}

bool Configure::loadConfigure()
{
    using namespace boost::property_tree;
    using namespace boost;

    if(!PATH::isExist(FILENAME))
    {
        std::cerr << "init-configure warning:\n"
            "    what: configure file is not exists.\n"
            "    file: " << FILENAME << "\n"
            "    approach: use default value.\n"
            << std::flush;
        return true;
    }

    ptree tree;
    try
        {read_json(FILENAME, tree);}
    catch(const std::exception &e)
    {
        std::cerr << "read-configure-file warning:\n"
            "    what: " << e.what() << "\n"
            "    file: " << FILENAME << "\n"
            "    approach: use default value.\n"
            << std::flush;
        return true;
    }

    bool isSuccessful = true;

    if(auto opt = tree.get_optional<std::string>(REPOSITORIES_JSON_FILE_KEY); opt)
        REPOSITORIES_JSON_FILE = opt.get();
    else
        isSuccessful = false;

    if(auto opt = tree.get_optional<std::string>(REPOSITORIES_DIR_KEY); opt)
        REPOSITORIES_DIR = opt.get();
    else
        isSuccessful = false;

    if(auto opt = tree.get_optional<std::string>(DIFFERENCE_DIR_KEY); opt)
        DIFFERENCE_DIR = opt.get();
    else
        isSuccessful = false;

    if(!isSuccessful)
    {
        std::cerr << "read-configure-file warning:\n"
            "    what: failed to read some value.\n"
            "    file: " << FILENAME << "\n"
            "    approach: " << "use default value.\n"
            << std::flush;
    }

    return true;
}

bool Configure::loadRepositories()
{
    using namespace boost::property_tree;
    using namespace boost;

    REPOSITORIES_MAP.clear();

    if(!PATH::isExist(REPOSITORIES_JSON_FILE))
    {
        std::cerr << "load-repositories error:\n"
            "    what: repository list file is not exists.\n"
            "    file: " << REPOSITORIES_JSON_FILE
            << std::endl;
        return false;
    }

    ptree tree;
    try
        {read_json(REPOSITORIES_JSON_FILE, tree);}
    catch(const std::exception &e)
    {
        std::cerr << "read-repositories-json error:\n"
            "    what: " << e.what() << "\n"
            "    file: " << REPOSITORIES_JSON_FILE
            << std::endl;
        return false;
    }

    if(auto optarr = tree.get_child_optional(REPOSITORIES_KEY); optarr)
    {
        bool isSuccessful = true;
        for(auto &&c : optarr.get())
        {
            auto optname = c.second.get_optional<std::string>(REPOSITORIES_NAME_KEY);
            auto opturl = c.second.get_optional<std::string>(REPOSITORIES_URL_KEY);
            if(optname && opturl)
            {
                auto [iter, isValid] = REPOSITORIES_MAP.emplace(optname.get(), opturl.get());
                if(!isValid)
                {
                    std::cerr << "load-repositories warning:\n"
                        "    what: duplicate repository name.\n"
                        "    name: " << iter->first << "\n"
                        "    approach: use below url.\n"
                        "    url: " << iter->second
                        << std::endl;
                }
            }
            else
                isSuccessful = false;
        }

        if(!isSuccessful)
        {
            std::cerr << "load-repositories warning:\n"
                "    what: failed to get some repositories.\n"
                "    file: " << REPOSITORIES_JSON_FILE << "\n"
                "    approach: " << "ignore some repositories.\n"
                << std::flush;
        }
    }
    else
    {
        std::cerr << "load-repositories error:\n"
            "    what: failed to get repository-list.\n"
            "    file: " << REPOSITORIES_JSON_FILE
            << std::endl;
        return false;
    }

    return true;
}