#include "luaproxy.h"
#include "log.h"

LuaProxy::LuaProxy(const string& name) :
    _name(name)
{
}

LuaProxy::~LuaProxy()
{
}

const string& LuaProxy::name() const
{
    return _name;
}

int lua_proxy_static_caller(lua_State* lua)
{
    Value caller(lua, lua_upvalueindex(1));
    auto _this = caller.get("instance").toPointer();
    auto methodName = caller.get("name").toString();

    ValuePack pack(lua);

    if (_this)
    {
        size_t top = lua_gettop(lua);
        for (size_t index = 1; index <= top; index++)
        {
            pack.push_back(Value(lua, index));
        }

        LuaProxy* p = reinterpret_cast<LuaProxy*>(_this);
        pack = p->invoke(methodName, pack);

        // settop(0) here to save memory but it is NOT necessary
        lua_settop(lua, 0);

        // push pack result on stack
        for (size_t index = 0; index < pack.size(); index++)
        {
            pack.at(index).push(lua);
        }
    }

    return pack.size();
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

ValuePack LuaProxy::invoke(const string& methodName, const ValuePack& args)
{
    lout << "LuaProxy." << _name << "." << methodName << "(" << args << ") -> ";
    const auto& mit = _methods.find(methodName);
    if (mit == _methods.end())
    {
        lout << "no such method\n";
        luaL_error(args.state, "no such method: %s", methodName.c_str());
    }

    ProxyMethod pmethod = mit->second;
    auto result = ((*this).*pmethod)(args);
    lout << result << endl;
    return result;
}
