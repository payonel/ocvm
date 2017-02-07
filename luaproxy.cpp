#include "luaproxy.h"
#include "log.h"

using std::vector;
using std::string;

LuaProxy::LuaProxy(const std::string& name) :
    _name(name)
{
}

LuaProxy::~LuaProxy()
{
}

const std::string& LuaProxy::name() const
{
    return _name;
}

// int _a(lua_State* lua)
// {
//     Value udata_call = Value::make(lua, 1);

//     ValuePack pack;
//     pack.push_back(Value::make(lua, 2));
//     pack.push_back(Value::make(lua, 3));

//     const Value& mt = udata_call.metatable();
//     if (mt)
//     {
//         const Value& inst = mt.get("instance");
//         if (inst.type() == "userdata")
//         {
//             void* p = inst.toPointer();
//             Client* pc = static_cast<Client*>(p);
//             //vector<Component*> comps = 
//             pc->component_list(pack);
//         }
//     }

//     return 0;
// }

int lua_proxy_static_caller(lua_State* lua)
{
    lout << "from _b\n";

    auto caller = Value::make(lua, 1);
    auto _this = caller.metatable().get("instance").toPointer();
    auto methodName = caller.get("name").toString();

    ValuePack pack;

    if (_this)
    {
        size_t top = lua_gettop(lua);
        for (size_t index = 2; index <= top; index++)
        {
            pack.push_back(Value::make(lua, index));
        }

        LuaProxy* p = reinterpret_cast<LuaProxy*>(_this);
        pack = p->invoke(methodName, pack);

        // push pack result on stack
    }

    return 0;
}

vector<LuaMethod> LuaProxy::methods() const
{
    vector<LuaMethod> result;
    for (const auto& pair : _methods)
    {
        result.push_back(std::make_tuple(pair.first, &lua_proxy_static_caller));
    }
    return result;
}

void LuaProxy::add(const string& methodName, ProxyMethod method)
{
    _methods[methodName] = method;
}

ValuePack LuaProxy::invoke(const std::string& methodName, const ValuePack& args)
{
    const auto& mit = _methods.find(methodName);
    if (mit != _methods.end())
    {
        ProxyMethod pmethod = mit->second;
        return ((*this).*pmethod)(args);
    }

    return ValuePack();
}