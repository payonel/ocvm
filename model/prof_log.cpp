#include "prof_log.h"

#include <sstream>
#include <fstream>
using std::ofstream;
using std::fstream;
using std::endl;
using std::stringstream;

ofstream open_file(const string& filename, bool append)
{
    return std::move(ofstream(filename, append ? fstream::app : fstream::out));
}

bool ProfLog::open(const string& dump_file)
{
    _filename = "";
    if (dump_file.empty())
        return false;

    ofstream ofs = open_file(dump_file, false);
    bool ok = ofs.is_open();
    if (ok)
        _filename = dump_file;
    ofs.close();
    return ok;
}

bool ProfLog::is_open()
{
    return !_filename.empty();
}

ProfLog::~ProfLog()
{
    flush();
}

void ProfLog::release(void* ptr)
{
    stringstream ss;
    ss << ptr << " " << 0 << endl;
    push(ss.str());
}

void ProfLog::trace(const string& stacktrace, void* ptr, size_t size)
{
    stringstream ss;
    ss << ptr << " " << size << " " << stacktrace << endl;
    push(ss.str());
}

void ProfLog::flush()
{
    ofstream ofs = open_file(_filename, true);
    for (const auto& text : _buffer)
        ofs << text;
    ofs.close();
    _buffer.clear();
}

void ProfLog::push(const string& text)
{
    _buffer.push_back(text);

    if (_buffer.size() > 10000)
    {
        flush();
    }
}

