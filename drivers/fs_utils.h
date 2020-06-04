#pragma once

#include <functional>
#include <string>
#include <vector>
using std::function;
using std::string;
using std::vector;

namespace fs_utils
{
bool read(const string& path, vector<char>& outData);
bool read(const string& path, string* pOutData = nullptr);
bool copy(const string& src, const string& dst);
bool write(const string& data, const string& dst);
bool write(const vector<char>& data, const string& dst);

bool mkdir(const string& path);
bool exists(const string& path);

vector<string> list(const string& path);
bool isDirectory(const string& path);

size_t size(const string& path, bool recursive = false);
uint64_t lastModified(const string& path);

bool remove(const string& path);
bool rename(const string& from, const string& to);
bool resize(const string& path, int size);

bool run_safely(function<void()> func, function<void(const string&)> onError = nullptr);

string make_proc_path(const string& given_path);
string make_pwd_path(const string& given_path);
void set_prog_name(const string& prog_name);

string filename(const string& path);
};
