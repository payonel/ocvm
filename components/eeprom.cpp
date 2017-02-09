#include "eeprom.h"
#include "log.h"
#include <iostream>
#include <string>
#include <fstream>
#include "config.h"

using std::ifstream;
using std::ofstream;

Eeprom::Eeprom(const string& type, const Value& init) :
    Component(type, init)
{
    add("get", &Eeprom::get);

    load(init.get("env").toString(), init.get(2).toString());
}

void Eeprom::load(const string& dir, const string& file)
{
    if (dir.empty())
    {
        lout << "bug, eeprom env dir path empty\n";
        return;
    }

    _path = dir + "/" + file;

    // if _path doesn't exist, copy from system/${file]
    ifstream f(_path);
    if (!f)
    {
        f.open("system/" + file);
        if (!f)
        {
            lout << "eeprom could not open system source file\n";
            return;
        }

        Config eepromConfig;
        eepromConfig.load(dir, file);

        string bios;
        char byte;
        while (byte = f.get())
        {
            if (!f)
                break;
            bios += byte;
        }
        f.close();

        eepromConfig.set("content", bios);
        eepromConfig.set("data", "");
        if (!eepromConfig.save())
        {
            lout << "could not create eeprom storage\n";
        }
    }
}

ValuePack Eeprom::get(const ValuePack& args)
{
    return ValuePack();
}
