#include "data.h"

#include <limits>

#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1

#include <cryptopp/md5.h>
#include <cryptopp/crc.h>
#include <cryptopp/filters.h>
#include <cryptopp/base64.h>
#include <cryptopp/zdeflate.h>
#include <cryptopp/zinflate.h>
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>

using namespace CryptoPP;

Data::Data()
{
    add("crc32", &Data::crc32);
    add("encode64", &Data::encode64);
    add("decode64", &Data::decode64);
    add("md5", &Data::md5);
    add("deflate", &Data::deflate);
    add("inflate", &Data::inflate);
    add("getLimit", &Data::getLimit);
    add("encrypt", &Data::encrypt);
    add("decrypt", &Data::decrypt);
}

Data::~Data()
{
    
}

bool Data::onInitialize()
{
    return true;
}

int Data::crc32(lua_State* lua)
{
    string input = Value::checkArg<string>(lua, 1);
    string result;

    CRC32 hash;
    StringSource(input, true,
                 new HashFilter(hash,
                                new StringSink(result)));

    return ValuePack::ret(lua, result);
}

int Data::encode64(lua_State* lua)
{
    string input = Value::checkArg<string>(lua, 1);
    string result;

    StringSource(input, true,
                 new Base64Encoder(
                     new StringSink(result), false));

    return ValuePack::ret(lua, result);
}

int Data::decode64(lua_State* lua)
{
    string input = Value::checkArg<string>(lua, 1);
    string result;

    StringSource(input, true,
                 new Base64Decoder(
                     new StringSink(result)));

    return ValuePack::ret(lua, result);
}

int Data::md5(lua_State* lua)
{
    string input = Value::checkArg<string>(lua, 1);
    string result;

    Weak::MD5 hash;
    StringSource(input, true,
                 new HashFilter(hash,
                                new StringSink(result)));

    return ValuePack::ret(lua, result);
}

int Data::deflate(lua_State* lua)
{
    string input = Value::checkArg<string>(lua, 1);
    string result;

    StringSource(input, true,
		 new Deflator(
		     new StringSink(result)));

    return ValuePack::ret(lua, result);
}

int Data::inflate(lua_State* lua)
{
    string input = Value::checkArg<string>(lua, 1);
    string result;

    StringSource(input, true,
		 new Inflator(
		     new StringSink(result)));

    return ValuePack::ret(lua, result);
}

int Data::getLimit(lua_State* lua)
{
    /* set to the default hardlimit, no config option yet */
    return ValuePack::ret(lua, 1048576);
}

int Data::encrypt(lua_State* lua)
{
    string input = Value::checkArg<string>(lua, 1);
    string key = Value::checkArg<string>(lua, 2);
    string iv = Value::checkArg<string>(lua, 3);
    string result;

    if (key.length() != 16)
	luaL_error(lua, "expected a 128-bit AES key");
    if (iv.length() != 16)
	luaL_error(lua, "expected a 128-bit AES IV");

    CBC_Mode<AES>::Encryption cipher((const byte *) key.c_str(),
				     key.length(),
				     (const byte *) iv.c_str());

    StringSource(input, true,
		 new StreamTransformationFilter(cipher,
						new StringSink(result),
						StreamTransformationFilter::PKCS_PADDING));

    return ValuePack::ret(lua, result);
}

int Data::decrypt(lua_State *lua)
{
    string input = Value::checkArg<string>(lua, 1);
    string key = Value::checkArg<string>(lua, 2);
    string iv = Value::checkArg<string>(lua, 3);
    string result;

    if (key.length() != 16)
	luaL_error(lua, "expected a 128-bit AES key");
    if (iv.length() != 16)
	luaL_error(lua, "expected a 128-bit AES IV");

    CBC_Mode<AES>::Decryption cipher((const byte *) key.c_str(),
				     key.length(),
				     (const byte *) iv.c_str());

    StringSource(input, true,
		 new StreamTransformationFilter(cipher,
						new StringSink(result),
						StreamTransformationFilter::PKCS_PADDING));

    return ValuePack::ret(lua, result);

}
