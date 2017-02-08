#include "eeprom.h"
#include "log.h"
#include <iostream>
#include <string>
using std::string;
using std::endl;

Eeprom::Eeprom(const string& type, const Value& init) :
    Component(type, init)
{
}
