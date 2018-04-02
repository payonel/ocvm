#pragma once
#include "component.h"

#include "model/value.h"

class Data : public Component
{
public:
    /* tier 1 */
    int crc32(lua_State*);
    int encode64(lua_State*);
    int decode64(lua_State*);
    int md5(lua_State*);
    int sha256(lua_State*);
    int deflate(lua_State*);
    int inflate(lua_State*);
    int getLimit(lua_State*);

    /* tier 2 */
    int encrypt(lua_State*);
    int decrypt(lua_State*);
    int random(lua_State*);

    /* tier 3 */
    int generateKeyPair(lua_State*);
    int ecdsa(lua_State*);
    int ecdh(lua_State*);
    int deserializeKey(lua_State*);
    
    Data();
    ~Data();
protected:
    bool onInitialize() override;
};
