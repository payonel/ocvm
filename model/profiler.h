#pragma once

#include <string>
#include <vector>
using std::string;
using std::vector;

struct lua_State;

struct CallNode
{
    int64_t memory = 0;
    string name;
    vector<CallNode> children;
};

class Profiler
{
public:
    void trace(size_t osize, size_t nsize, lua_State* lua);
    void dump(const std::string& dump_file);
private:
    void store_snapshot();
    string serialize_calls(const CallNode* pNode, int64_t* pMem, string tab = "");
    void locked_trace(size_t osize, size_t nsize, lua_State* lua);

    CallNode _root;
    vector<string> _snaps;
    bool _locked = false;
};
