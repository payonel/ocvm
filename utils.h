#pragma once

#include <string>
#include <vector>
using std::string;
using std::vector;

namespace utils
{
    bool read(const string& path, string* pOutData = nullptr);
    bool copy(const string& src, const string& dst);
    bool write(const string& data, const string& dst);

    void mkdir(const string& path);
    bool exists(const string& path);

    vector<string> list(const string& path);
    bool isDirectory(const string& path);
};

