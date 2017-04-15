#pragma once

#include <string>
#include <vector>
using std::string;
using std::vector;

class ProfLog
{
public:
    ~ProfLog();
    void release(void* ptr);
    void trace(const string& stacktrace, void* ptr, size_t size);
    void flush();
    bool open(const string& filename);
    bool is_open();
private:
    void push(const string& text);
    vector<string> _buffer;
    string _filename;
};
