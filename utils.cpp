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

    error_code ec;
    fs::copy(src, dst, fs::copy_options::recursive, ec);
    return ec.value() == 0;
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
    error_code ec;
    fs::create_directories(path, ec);
}

bool utils::exists(const string& path)
{
    error_code ec;
    bool result = fs::exists(path, ec);
    return result && ec.value() == 0;
}

vector<string> utils::list(const string& path)
{
    vector<string> result;
    if (!utils::exists(path))
        return result;

    error_code ec;
    for (const auto& ele : fs::directory_iterator(path, ec))
    {
        if (ec.value() != 0)
            return {};

        result.push_back(ele.path());
    }

    return result;
}

bool utils::isDirectory(const string& path)
{
    error_code ec;
    bool result = fs::is_directory(path, ec);
    return result && ec.value() == 0;
}

size_t utils::size(const string& path, bool recursive)
{
    error_code ec;
    size_t result = 0;
    if (!utils::isDirectory(path))
    {
        result = fs::file_size(path, ec);
        if (ec.value() != 0)
            result = 0;
    }
    return result;
}

uint64_t utils::lastModified(const string& filepath)
{
    error_code ec;
    uint64_t result = 0;

    auto ftime = fs::last_write_time(filepath, ec);
    if (ec.value() != 0)
        return 0;
    
    std::time_t cftime = decltype(ftime)::clock::to_time_t(ftime);
    result = static_cast<uint64_t>(cftime);

    return result;
}

bool utils::remove(const string& path)
{
    bool success = true;
    error_code ec;
    auto info = fs::symlink_status(path, ec);
    if (ec.value() == 0 && info.type() == fs::file_type::directory)
        for (fs::directory_iterator d(path, ec), end; ec.value() == 0 && d != end; ++d)
            success = success && utils::remove(d->path());
    if (ec.value())
        return -1;
    success = success && fs::remove(path, ec);
    return success && ec.value() == 0;
}

bool utils::rename(const string& from, const string& to)
{
    error_code ec;
    fs::rename(from, to, ec);
    return ec.value() == 0;
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
