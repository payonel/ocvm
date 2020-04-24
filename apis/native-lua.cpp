#include "native-lua.h"

int ocvm_resume(lua_State *L, lua_State *from, int narg, int *nres)
{
  assert(nres != nullptr);
#if LUA_VERSION_NUM == 504
  return lua_resume(L, from, narg, nres);
#else
  int status = lua_resume(L, from, narg);
  *nres = lua_gettop(L);
  return status;
#endif
}
