#pragma once
#include "component.h"
#include "value.h"
#include <tuple>
using std::tuple;

class Screen;

class Gpu : public Component
{
public:
    Gpu();

    bool set(int x, int y, const string& text);

    ValuePack setResolution(lua_State* lua);
    ValuePack bind(lua_State* lua);
    ValuePack set(lua_State* lua);
    ValuePack maxResolution(lua_State* lua);
    ValuePack setBackground(lua_State* lua);
    ValuePack getBackground(lua_State* lua);
    ValuePack setForeground(lua_State* lua);
    ValuePack getForeground(lua_State* lua);
    ValuePack fill(lua_State* lua);
    ValuePack copy(lua_State* lua);
protected:
    bool onInitialize(Value& config) override;
    bool truncateWH(int x, int y, int* pWidth, int* pHeight) const;
private:
    Screen* _screen;
};
