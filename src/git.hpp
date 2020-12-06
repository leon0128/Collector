#ifndef GIT_HPP
#define GIT_HPP

#include <filesystem>
#include <string>
#include <utility>

#include <boost/property_tree/ptree.hpp>

namespace GIT
{

class Repository
{
public:
    Repository(const std::filesystem::path &p
        , const std::string &u)
        : mPath(p)
        , mUrl(u){}

    bool clone() const;
    bool pull() const;
    bool log(const std::filesystem::path &logpath) const;
    bool diff(const std::filesystem::path &input
        , const std::filesystem::path &output) const;

    void remove(const std::filesystem::path &diffdir) const;
    bool setUrl();

    const std::filesystem::path &path() const noexcept
        {return mPath;}
    const std::string &url() const noexcept
        {return mUrl;}

private:
    bool show(const std::filesystem::path &output
        , const std::string &hash) const;
    bool parseShow(const std::filesystem::path &showpath
        , boost::property_tree::ptree&) const;

    bool outputDiff(const std::filesystem::path &output
        , const std::string &hash
        , const std::string &subject) const;

    bool outSystemError(const std::string &cmd) const;
    bool outFileError(const std::filesystem::path&) const;

    std::filesystem::path mPath;
    std::string mUrl;
};

}

#endif