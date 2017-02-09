#pragma once

#include <string>
using std::string;

namespace utils
{
    bool read(const string& path, string* pOutData = nullptr);
    bool copy(const string& src, const string& dst);
    bool write(const string& data, const string& dst);
};

