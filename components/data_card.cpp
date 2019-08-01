#include "data_card.h"

#include "model/host.h"
#include "model/log.h"

#include "util/crc32.h"
#include "util/md5.h"

bool DataCard::s_registered = Host::registerComponentType<DataCard>("data");

DataCard::DataCard()
{
    add("crc32", &DataCard::crc32);
    add("md5", &DataCard::md5);
}

DataCard::~DataCard(){};

bool DataCard::onInitialize()
{
    int config_tier = config().get(ConfigIndex::Tier).toNumber();

    _tier = config_tier == 0 ? _tier : config_tier;

    return true;
}

int DataCard::crc32(lua_State* lua)
{
    vector<char> value = Value::checkArg<vector<char>>(lua, 1);

    return ValuePack::ret(lua, util::crc32(value));
}

int DataCard::md5(lua_State* lua)
{
    vector<char> value = Value::checkArg<vector<char>>(lua, 1);

    return ValuePack::ret(lua, util::md5(value));
}