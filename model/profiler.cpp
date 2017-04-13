#include "profiler.h"
#include "value.h"

#include <sstream>
#include <fstream>
#include <chrono>
using namespace std::chrono;

using namespace std;

static const string massif_separator = "#-----------";

string Profiler::serialize_calls(const CallNode* pNode, int64_t* pMem, string tab)
{
    string result;
    // depth first
    for (const auto& child : pNode->children)
    {
        int64_t mem = 0;
        string line = serialize_calls(&child, &mem, tab + " ");
        *pMem += mem;
        result += line;
    }

    *pMem += pNode->memory;
    int64_t mem = std::max(static_cast<int64_t>(0), *pMem);

    stringstream ss;
    ss << tab << "n" << pNode->children.size() << ": ";
    ss << mem << " ";
    ss << pNode->name << endl;
    ss << result;
    
    return ss.str();
}

void Profiler::store_snapshot()
{
/*
snapshot=0
#-----------
time=0
mem_heap_B=0
mem_heap_extra_B=0
mem_stacks_B=0
heap_tree=detailed
n1: 0 new[]
 n0: 0 main
*/
    // auto now = system_clock::now().time_since_epoch() / nanoseconds(1) % 1492093709000000000ull;
    int64_t mem_total = 0;
    string heap_tree = serialize_calls(&_root, &mem_total);
    mem_total = std::max(static_cast<int64_t>(0), mem_total);

    stringstream ss;
    ss << "snapshot=" << _snaps.size() << endl;
    ss << massif_separator << endl;
    ss << "time=" << _snaps.size() << endl;
    ss << "mem_heap_B=" << mem_total << endl;
    ss << "mem_heap_extra_B=0\n";
    ss << "mem_stacks_B=0\n";
    ss << "heap_tree=detailed\n";

    ss << heap_tree; // tree print already has newline

    _snaps.push_back(ss.str());
}

static vector<string> parse_calls(const string& stack_text)
{
    vector<string> lines;
    stringstream ss(stack_text);
    while (ss)
    {
        string full_line;
        getline(ss, full_line);

        // ignore starting "stack trace:"
        if (full_line == "stack traceback:")
            continue;

        // ignore empty lines
        if (full_line.empty())
            continue;

        // remove leading \t
        if (full_line.at(0) == '\t')
            full_line = full_line.substr(1);

        // ignore machine calls
        if (full_line.find("system/machine.lua") == 0)
            continue;

        lines.push_back(full_line);
    }
    return lines;
}

static CallNode* find_node(lua_State* lua, CallNode* pRoot)
{
    if (lua == nullptr)
        return nullptr;

    string stack = Value::stack(lua);
    vector<string> calls = parse_calls(stack);
    auto call_it = calls.begin();
    if (call_it == calls.end())
        return nullptr;

    CallNode* node = pRoot;
    while (call_it != calls.end())
    {
        CallNode* next = nullptr;
        for (CallNode& child : node->children)
        {
            if (*call_it == child.name)
            {
                next = &child;
                break;
            }
        }

        if (!next)
        {
            CallNode add;
            add.name = *call_it;
            node->children.push_back(add);
            next = &(node->children.at(node->children.size() - 1));
        }
        call_it++;
        node = next;
    }
    
    return node;
}

void Profiler::trace(size_t osize, size_t nsize, lua_State* lua)
{
    if (_locked)
        return; // ignore, stack inspection caused allocation
    _locked = true;
    locked_trace(osize, nsize, lua);
    _locked = false;
}

void Profiler::locked_trace(size_t osize, size_t nsize, lua_State* lua)
{
    if (_snaps.size() > 10)
        return;
    
    lua_State* coState = nullptr;
    lua_Debug ar;
    lua_getstack(lua, 1, &ar);
    for (int n = 1; n < 10 && !coState; n++)
    {
        const char* cstrVarName = lua_getlocal(lua, &ar, n);
        if (!cstrVarName)
        {
            break; // could not find coState
        }
        string varname = cstrVarName;
        if (varname == "co")
        {
            coState = lua_tothread(lua, -1);
        }
        lua_pop(lua, 1);
    }

    if (!coState)
        return;

    CallNode* node = find_node(coState, &_root);

    if (node == nullptr)
        return;

    node->memory += nsize;
    node->memory -= osize;

    store_snapshot();
}

void Profiler::dump(const string& dumpfile)
{
    ofstream ofs(dumpfile);
/*
desc: --time-unit=B
cmd: ./a.out
time_unit: B
#-----------
*/
    ofs << "desc: (none)\n";
    ofs << "cmd: ./ocvm\n";
    ofs << "time_unit: i\n";

    for (const auto& snap : _snaps)
    {
        ofs << massif_separator << endl;
        ofs << snap;
    }

    ofs.close();
}
