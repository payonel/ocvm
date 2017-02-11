#include "unicode.h"

UnicodeApi::UnicodeApi() : LuaProxy("unicode")
{
}

UnicodeApi* UnicodeApi::get()
{
    static UnicodeApi it;
    return &it;
}
