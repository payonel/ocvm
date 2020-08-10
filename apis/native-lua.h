#pragma once

// lua.hpp
// Lua header files for C++
// <<extern "C">> not supplied automatically because Lua also compiles as C++

#if __has_include("lua.hpp")
#include <lua.hpp>
#else
extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
#endif

#undef NEDBUG
#include <cassert>

// lua 5.3 -> 5.4 changes lua_resume

struct lua_State;
int  ocvm_resume(lua_State *L, lua_State *from, int narg, int *nres);

