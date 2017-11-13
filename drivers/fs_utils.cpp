#include "drivers/fs_utils.h"

#include "model/log.h"
#include <iostream>
#include <fstream>
using std::cerr;
using std::ifstream;
using std::ofstream;
using std::error_code;

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#include <unistd.h>

#include <sys/stat.h>
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#include <wordexp.h>

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

string resolve(const string& path)
{
    string result = path;
    if (path.size() > 0 && path.find("~") == 0)
    {
        ::wordexp_t exp_result;
        if (::wordexp(path.c_str(), &exp_result, 0) == 0)
        {
            if (exp_result.we_wordc == 1)
            {
                result = std::string(exp_result.we_wordv[0]);
            }
            ::wordfree(&exp_result);
        }
    }

    return result;
}

bool fs_utils::run_safely(function<void()> func, function<void(const string&)> onError)
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

bool fs_utils::read(const string& path, vector<char>& outData)
{
    ifstream file;
    file.open(resolve(path));
    if (!file)
        return false;

    std::filebuf* pbuf = file.rdbuf();
    std::size_t buffer_size = pbuf->pubseekoff(0, file.end, file.in);
    pbuf->pubseekpos(0, file.in);

    outData.resize(buffer_size);
    pbuf->sgetn(outData.data(), buffer_size);

    file.close();
    return true;
}

bool fs_utils::read(const string& path, string* pOutData)
{
    ifstream file;
    file.open(resolve(path));
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

bool fs_utils::copy(const string& src, const string& dst)
{
    string resolved_src = resolve(src);
    string resolved_dst = resolve(dst);
    if (!fs_utils::exists(resolved_src))
    {
        return false;
    }

    error_code ec;
    fs::copy(resolved_src, resolved_dst, fs::copy_options::recursive, ec);
    return ec.value() == 0;
}

bool fs_utils::write(const vector<char>& data, const string& dst)
{
    ofstream file;
    file.open(resolve(dst));
    if (!file)
        return false;
    
    file.write(data.data(), data.size());
    file.close();

    return true;
}

bool fs_utils::write(const string& data, const string& dst)
{
    vector<char> buffer { data.begin(), data.end() };
    return fs_utils::write(buffer, dst);
}

void fs_utils::mkdir(const string& path)
{
    error_code ec;
    fs::create_directories(path, ec);
}

bool fs_utils::exists(const string& path)
{
    error_code ec;
    bool result = fs::exists(path, ec);
    return result && ec.value() == 0;
}

vector<string> fs_utils::list(const string& path)
{
    vector<string> result;
    if (!fs_utils::exists(path))
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

bool fs_utils::isDirectory(const string& path)
{
    error_code ec;
    bool result = fs::is_directory(path, ec);
    return result && ec.value() == 0;
}

size_t fs_utils::size(const string& path, bool recursive)
{
    error_code ec;
    size_t result = 0;
    if (!fs_utils::isDirectory(path))
    {
        result = fs::file_size(path, ec);
        if (ec.value() != 0)
            result = 0;
    }
    return result;
}

uint64_t fs_utils::lastModified(const string& filepath)
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

bool fs_utils::remove(const string& path)
{
    bool success = true;
    error_code ec;
    auto info = fs::symlink_status(path, ec);
    if (ec.value() == 0 && info.type() == fs::file_type::directory)
        for (fs::directory_iterator d(path, ec), end; ec.value() == 0 && d != end; ++d)
            success = success && fs_utils::remove(d->path());
    if (ec.value())
        return -1;
    success = success && fs::remove(path, ec);
    return success && ec.value() == 0;
}

bool fs_utils::rename(const string& from, const string& to)
{
    error_code ec;
    fs::rename(from, to, ec);
    return ec.value() == 0;
}

string proc_root()
{
    static string path;
    if (path.empty())
    {
        constexpr ssize_t size = 1024;
        char buf[size];

#ifdef __APPLE__
        uint32_t len = size;
        if(_NSGetExecutablePath(buf, &len) != 0) {
            cerr << "proc path too long\n";
            ::exit(1);
        }
#else
        ssize_t len = ::readlink("/proc/self/exe", buf, size);
        if (len >= size) // yikes, abort
        {
            cerr << "proc path too long\n";
            ::exit(1);
        }
#endif

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

string pwd()
{
    error_code ec;
    string path = fs::current_path(ec);
    return path + "/";
}

string fs_utils::make_proc_path(const string& given_path)
{
    static string path = proc_root();
    if (given_path.substr(0, 1) != "/")
    {
        return path + given_path;
    }
    return given_path;
}

string fs_utils::make_pwd_path(const string& given_path)
{
    static string pwd_path = pwd();
    if (given_path.substr(0, 1) != "/")
    {
        return pwd_path + given_path;
    }
    return given_path;
}
