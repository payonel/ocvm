#pragma once

#include <string>
#include <vector>
#include <functional>
using std::string;
using std::vector;
using std::function;

namespace fs_utils
{
    bool read(const string& path, vector<char>& outData);
    bool read(const string& path, string* pOutData = nullptr);
    bool copy(const string& src, const string& dst);
    bool write(const string& data, const string& dst);
    bool write(const vector<char>& data, const string& dst);

    void mkdir(const string& path);
    bool exists(const string& path);

    vector<string> list(const string& path);
    bool isDirectory(const string& path);

    size_t size(const string& path, bool recursive = false);
    uint64_t lastModified(const string& path);

    bool remove(const string& path);
    bool rename(const string& from, const string& to);

    bool run_safely(function<void()> func, function<void(const string&)> onError = nullptr);
    string proc_root();
};

