#include "utils.h"

#include "log.h"
#include <iostream>
#include <fstream>
using namespace std;

#include <unistd.h>

#include <sys/stat.h>
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

static string handle_exception(std::exception& exp)
{
    return exp.what();
}

static string handle_exception(std::exception_ptr&& p)
{
    try
    {
        if (p)
            std::rethrow_exception(p);
    }
    catch (std::exception& exp)
    {
        return handle_exception(exp);
    }

    return "unknown exception";
}

bool utils::run_safely(function<void()> func, function<void(const string&)> onError)
{
    string exception_message;
    try
    {
        func();
        return true;
    }
    catch (...)
    {
        exception_message = handle_exception(std::current_exception());
    }

    lout << exception_message << std::endl;
    if (onError)
        onError(exception_message);

    return false;
}

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

    return run_safely([&src, &dst]()
    {
        fs::copy(src, dst, fs::copy_options::recursive);
    });
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
    run_safely([&path](){fs::create_directories(path);});
}

bool utils::exists(const string& path)
{
    bool result = false;
    run_safely([&result, &path](){result = fs::exists(path);});
    return result;
}

vector<string> utils::list(const string& path)
{
    vector<string> result;
    if (!utils::exists(path))
        return result;

    run_safely([&path, &result]()
    {
        for (const auto& ele : fs::directory_iterator(path))
        {
            result.push_back(ele.path());
        }
    });

    return result;
}

bool utils::isDirectory(const string& path)
{
    bool result = false;
    run_safely([&result, &path]()
    {
        result = utils::exists(path) && fs::is_directory(path);
    });
    return result;
}

size_t utils::size(const string& path, bool recursive)
{
    size_t total = 0;
    run_safely([&total, &path, &recursive]()
    {
        if (utils::isDirectory(path))
        {
            if (recursive)
            {
                for (const auto& item : list(path))
                {
                    total += size(item, true);
                }
            }
        }
        else
        {
            total = fs::file_size(path);
        }
    });

    return total;
}

uint64_t utils::lastModified(const string& filepath)
{
    uint64_t result = 0;
    run_safely([&result, &filepath]()
    {
        auto ftime = fs::last_write_time(filepath);
        std::time_t cftime = decltype(ftime)::clock::to_time_t(ftime);
        result = static_cast<uint64_t>(cftime);
    });

    return result;
}

bool utils::remove(const string& path)
{
    bool success = false;
    run_safely([&success, &path]()
    {
        error_code ec;
        auto info = fs::symlink_status(path, ec);
        if (ec.value() == 0 && info.type() == fs::file_type::directory)
            for (fs::directory_iterator d(path, ec), end; ec.value() == 0 && d != end; ++d)
                success = success && utils::remove(d->path());
        if (ec.value())
            return -1;
        success = success && fs::remove(path, ec);
    });
    return success;
}

string utils::proc_root()
{
    static string path;
    if (path.empty())
    {
        constexpr ssize_t size = 1024;
        char buf[size];
        ssize_t len = ::readlink("/proc/self/exe", buf, size);
        if (len >= size) // yikes, abort
        {
            cerr << "proc path too long\n";
            ::exit(1);
        }
        buf[len] = 0;
        path = buf;

        // remove proc file name
        size_t last_slash = path.find_last_of("/");
        if (last_slash == string::npos)
        {
            cerr << "proc path had no dir slash, unexpected\n";
            ::exit(1);
        }
        path = path.substr(0, last_slash + 1);
    }
    return path;
}
