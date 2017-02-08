#include "eeprom.h"
#include "log.h"
#include <iostream>
#include <string>

Eeprom::Eeprom(const string& type, const Value& init) :
    Component(type, init)
{
    add("get", &Eeprom::get);

    lout << "eeprom init" << init.serialize() << endl;
}

ValuePack Eeprom::get(const ValuePack& args)
{
    return ValuePack();
}
