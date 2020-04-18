#include "os.h"

OSApi::OSApi()
    : LuaProxy("os")
{
}

OSApi* OSApi::get()
{
  static OSApi it;
  return &it;
}
