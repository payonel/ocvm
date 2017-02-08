#pragma once
#include "component.h"
#include "value.h"

class Eeprom : public Component
{
public:
    Eeprom(const string& type, const Value& init);

    ValuePack get(const ValuePack& args);
private:
    string _content;
    string _data;

    string _path; // real on disk storage location
};
