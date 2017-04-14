#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
using std::string;
using std::vector;
using std::map;
using std::set;

struct lua_State;

struct CallNode
{
    int64_t memory() const;
    string name;
    set<CallNode*> children;
    map<void*, int64_t> ptrs;
};

class Profiler
{
public:
    void trace(const string& stacktrace, void* ptr, size_t size);
    void release(void* ptr);
    void dump(const std::string& dump_file);
private:
    void locked_trace(const string& stacktrace, void* ptr, size_t size);
    void store_snapshot();
    string serialize_calls(const CallNode* pNode, int64_t* pMem, string tab = "");

    CallNode _root;
    map<void*, CallNode*> _ptrs;
    vector<string> _snaps;
    bool _locked = false;
};
