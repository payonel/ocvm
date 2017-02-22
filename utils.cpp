#include "utils.h"

#include "log.h"
#include <iostream>
#include <fstream>
using std::ifstream;
using std::ofstream;

#include <sys/stat.h>
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

bool utils::read(const string& path, string* pOutData)
{
    ifstream file;
    file.open(path);
    if (!file)
        return false;

    // read without p is just open test
    if (pOutData)
    {
        *pOutData = std::move(static_cast<stringstream const&>(stringstream() << file.rdbuf()).str());
    }

    file.close();
    return true;
}

bool utils::copy(const string& src, const string& dst)
{
    if (!utils::exists(src))
    {
        return false;
    }

    try
    {
        fs::copy(src, dst, fs::copy_options::recursive);
    }
    catch (std::exception& se)
    {
        lout << se.what() << std::endl;
        std::cerr << se.what() << std::endl;
        return false;
    }
    catch (...)
    {
        std::exception_ptr p = std::current_exception();
        std::cerr <<(p ? p.__cxa_exception_type()->name() : "null") << std::endl;
        return false;
    }

    return true;
}

bool utils::write(const string& data, const string& dst)
{
    ofstream file;
    file.open(dst);
    if (!file)
        return false;
    
    file << data;
    file.close();
    return true;
}

void utils::mkdir(const string& path)
{
    fs::create_directories(path);
}

bool utils::exists(const string& path)
{
    return fs::exists(path);
}

vector<string> utils::list(const string& path)
{
    vector<string> result;
    if (!utils::exists(path))
        return result;

    try
    {
        for (const auto& ele : fs::directory_iterator(path))
        {
            result.push_back(ele.path());
        }
    }
    catch (std::exception& se)
    {
        lout << se.what() << std::endl;
        std::cerr << se.what() << std::endl;
    }
    catch (...)
    {
        std::exception_ptr p = std::current_exception();
        std::cerr <<(p ? p.__cxa_exception_type()->name() : "null") << std::endl;
    }

    return result;
}

bool utils::isDirectory(const string& path)
{
    try
    {
        return utils::exists(path) && fs::is_directory(path);
    }
    catch (std::exception& se)
    {
        lout << se.what() << std::endl;
        std::cerr << se.what() << std::endl;
    }
    catch (...)
    {
        std::exception_ptr p = std::current_exception();
        std::cerr <<(p ? p.__cxa_exception_type()->name() : "null") << std::endl;
    }

    return false;
}

size_t utils::size(const string& path)
{
    try
    {
        if (utils::isDirectory(path))
            return 0;
        else
            return fs::file_size(path);
    }
    catch (std::exception& se)
    {
        lout << se.what() << std::endl;
        std::cerr << se.what() << std::endl;
    }
    catch (...)
    {
        std::exception_ptr p = std::current_exception();
        std::cerr <<(p ? p.__cxa_exception_type()->name() : "null") << std::endl;
    }

    return 0;
}

uint64_t utils::lastModified(const string& filepath)
{
    try
    {
        auto ftime = fs::last_write_time(filepath);
        std::time_t cftime = decltype(ftime)::clock::to_time_t(ftime);
        return static_cast<uint64_t>(cftime);
    }
    catch (std::exception& se)
    {
        lout << se.what() << std::endl;
        std::cerr << se.what() << std::endl;
    }
    catch (...)
    {
        std::exception_ptr p = std::current_exception();
        std::cerr <<(p ? p.__cxa_exception_type()->name() : "null") << std::endl;
    }

    return 0;
}

