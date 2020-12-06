#ifndef CONFIGURE_HPP
#define CONFIGURE_HPP

#include <filesystem>
#include <unordered_map>
#include <string>

class Configure;

class Configure
{
private:
    inline static const std::string FILENAME= "./configure.json";
    inline static const std::string REPOSITORIES_JSON_FILE_KEY = "repositories_json_file";
    inline static const std::string REPOSITORIES_DIR_KEY = "repositories_dir";
    inline static const std::string DIFFERENCE_DIR_KEY = "difference_dir";
    inline static std::filesystem::path REPOSITORIES_JSON_FILE = "./repositories.json";
    inline static std::filesystem::path REPOSITORIES_DIR = "./repositories";
    inline static std::filesystem::path DIFFERENCE_DIR = "./difference";

    inline static const std::string REPOSITORIES_KEY = "repositories";
    inline static const std::string REPOSITORIES_NAME_KEY = "name";
    inline static const std::string REPOSITORIES_URL_KEY = "url";
    inline static std::unordered_map<std::string, std::string> REPOSITORIES_MAP;

public:
    Configure() = delete;

    static bool initialize();
    static bool reloadRepositories();

    static const std::filesystem::path &repositoriesJsonFile() noexcept
        {return REPOSITORIES_JSON_FILE;}
    static const std::filesystem::path &repositoriesDir() noexcept
        {return REPOSITORIES_DIR;}
    static const std::filesystem::path &differenceDir() noexcept
        {return DIFFERENCE_DIR;}
    static const std::unordered_map<std::string, std::string> repositoriesMap() noexcept
        {return REPOSITORIES_MAP;};

private:
    static bool loadConfigure();
    static bool loadRepositories();
};

#endif