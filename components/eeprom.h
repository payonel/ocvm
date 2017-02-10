#pragma once
#include "component.h"
#include "value.h"

class Eeprom : public Component
{
public:
    Eeprom(const string& type, const Value& init, Host* host);

    ValuePack get(const ValuePack& args);
    ValuePack getData(const ValuePack& args);
protected:
    void init(const string& originalBiosPath);
    string biosPath() const;
    string dataPath() const;
    string load(const string& path) const;
private:
    string _bios;
    string _data;

    string _dir; // real on disk storage location
};
