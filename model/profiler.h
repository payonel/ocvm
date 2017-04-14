#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <memory>
#include <tuple>
using std::string;
using std::vector;
using std::map;
using std::set;
using std::unique_ptr;
using std::tuple;

struct lua_State;

struct CallNode
{
    int64_t memory() const;
    string name;
    set<unique_ptr<CallNode>> children;
    CallNode* parent = nullptr;
    map<void*, int64_t> ptrs;
};

class Profiler
{
public:
    Profiler();
    void trace(const string& stacktrace, void* ptr, size_t size);
    void release(void* ptr);
    void dump(const std::string& dump_file);
private:
    void locked_trace(const string& stacktrace, void* ptr, size_t size);
    void store_snapshot();
    string serialize_calls(const CallNode* pNode, int64_t* pMem, string tab = "");

    unique_ptr<CallNode> _root;
    map<void*, CallNode*> _ptrs;
    vector<tuple<int64_t, string>> _snaps;
    bool _locked = false;
};
