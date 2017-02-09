#pragma once

#include "component.h"

class Computer : public Component
{
public:
    Computer(const string& type, const Value& init);

    ValuePack realTime(const ValuePack& args);
    ValuePack setArchitecture(const ValuePack& args);
    ValuePack getArchitecture(const ValuePack& args);
    ValuePack getArchitectures(const ValuePack& args);
    ValuePack address(const ValuePack& args);
    ValuePack beep(const ValuePack& args);
    ValuePack getDeviceInfo(const ValuePack& args);
    ValuePack getProgramLocations(const ValuePack& args);
    ValuePack uptime(const ValuePack& args);
    ValuePack pushSignal(const ValuePack& args);
    ValuePack removeUser(const ValuePack& args);
    ValuePack addUser(const ValuePack& args);
    ValuePack setBootAddress(const ValuePack& args);
    ValuePack getBootAddress(const ValuePack& args);
    ValuePack isRobot(const ValuePack& args);
    ValuePack tmpAddress(const ValuePack& args);
    ValuePack freeMemory(const ValuePack& args);
    ValuePack totalMemory(const ValuePack& args);
    ValuePack energy(const ValuePack& args);
    ValuePack maxEnergy(const ValuePack& args);
private:
};
