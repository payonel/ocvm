#include "profiler.h"
#include "value.h"

#include <sstream>
#include <fstream>
#include <chrono>
using namespace std::chrono;

using namespace std;

struct CallNode
{
    int64_t memory() const;
    string name;
    set<CallNode*> children;
    CallNode* parent = nullptr;
    map<void*, int64_t> ptrs;
};

static const string massif_separator = "#-----------";

int64_t CallNode::memory() const
{
    int64_t sum = 0;
    for (const auto& it : ptrs)
        sum += it.second;
    return sum;
}

Profiler::Profiler()
{
    _root = new CallNode;
    _root->parent = nullptr;
}

string Profiler::serialize_calls(const CallNode* pNode, int64_t* pMem, string tab)
{
    string result;
    // depth first
    for (const auto& child : pNode->children)
    {
        int64_t mem = 0;
        string line = serialize_calls(child, &mem, tab + " ");
        *pMem += mem;
        result += line;
    }

    *pMem += pNode->memory();
    int64_t mem = *pMem;//std::max(static_cast<int64_t>(0), *pMem);

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
    string heap_tree = serialize_calls(_root, &mem_total);
    // mem_total = std::max(static_cast<int64_t>(0), mem_total);

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
    static const string stack_line = "stack traceback:";
    static const string machine_pattern = "system/machine.lua";
    static const string dots = "...";
    static const string c_call = "[C]: in global 'pcall'";

    string full_line;
    vector<string> lines;
    stringstream ss(stack_text);
    while (ss)
    {
        getline(ss, full_line);

        // ignore starting "stack trace:"
        if (full_line == stack_line)
            continue;

        // ignore empty lines
        if (full_line.empty())
            continue;

        // remove leading \t
        if (full_line.at(0) == '\t')
            full_line = full_line.substr(1);

        // ignore machine calls
        if (full_line.find(machine_pattern) == 0)
            continue;

        // ignore ... sequences
        if (full_line == dots)
            continue;

        if (full_line == c_call)
            continue;

        lines.push_back(full_line);
    }
    return lines;
}

static CallNode* find_node(const string& stacktrace, CallNode* pRoot)
{
    vector<string> calls = parse_calls(stacktrace);
    auto call_it = calls.begin();
    if (call_it == calls.end())
        return nullptr;

    CallNode* node = pRoot;
    while (call_it != calls.end())
    {
        CallNode* next = nullptr;
        for (CallNode* child : node->children)
        {
            if (*call_it == child->name)
            {
                next = child;
                break;
            }
        }

        if (!next)
        {
            CallNode* add = new CallNode;
            add->parent = node;
            add->name = *call_it;
            node->children.insert(add);
            next = add;
        }
        call_it++;
        node = next;
    }
    
    return node;
}

void Profiler::trace(const string& stacktrace, void* ptr, size_t size)
{
    if (_locked)
        return; // ignore, stack inspection caused allocation
    _locked = true;
    locked_trace(stacktrace, ptr, size);
    _locked = false;
}

static inline void cut_empty_children(CallNode* pNode)
{
    if (pNode == nullptr || pNode->ptrs.size() || pNode->children.size())
        return;

    CallNode* parent = pNode->parent;
    if (parent)
    {
        parent->children.erase(pNode);
        cut_empty_children(parent);
    }
    delete pNode;
}

void Profiler::release(void* ptr)
{
    if (!ptr)
        return;

    const auto& node_it = _ptrs.find(ptr);
    if (node_it != _ptrs.end())
    {
        CallNode* pNode = node_it->second;
        pNode->ptrs.erase(ptr);
        cut_empty_children(pNode);
        _ptrs.erase(node_it);
    }
}

void Profiler::locked_trace(const string& stacktrace, void* ptr, size_t size)
{
    if (_snaps.size() > 10000)
        return;
    
    if (stacktrace.empty())
        return;

    CallNode* node = find_node(stacktrace, _root);
    if (node == nullptr)
        return;

    _ptrs[ptr] = node;
    node->ptrs[ptr] = size;

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
