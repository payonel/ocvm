#include "computer.h"

#include <iostream>
#include <chrono>
using namespace std::chrono;

Computer::Computer(Value& config, Host* host) :
    Component(config, host)
{
    _start_time = now();

    add("realTime", &Computer::realTime);
    // add("setArchitecture", &Computer::setArchitecture);
    // add("getArchitecture", &Computer::getArchitecture);
    // add("getArchitectures", &Computer::getArchitectures);
    add("beep", &Computer::beep);
    // add("getDeviceInfo", &Computer::getDeviceInfo);
    // add("getProgramLocations", &Computer::getProgramLocations);
    add("uptime", &Computer::uptime);
    // add("pushSignal", &Computer::pushSignal);
    // add("removeUser", &Computer::removeUser);
    // add("addUser", &Computer::addUser);
    // add("setBootAddress", &Computer::setBootAddress);
    // add("getBootAddress", &Computer::getBootAddress);
    // add("isRobot", &Computer::isRobot);
    add("tmpAddress", &Computer::tmpAddress);
    // add("freeMemory", &Computer::freeMemory);
    // add("totalMemory", &Computer::totalMemory);
    // add("energy", &Computer::energy);
    // add("maxEnergy", &Computer::maxEnergy);
}

int64_t Computer::now() const
{
    return duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
}

ValuePack Computer::realTime(const ValuePack& args)
{
    return ValuePack{now()};
}

ValuePack Computer::setArchitecture(const ValuePack& args)
{
    return ValuePack();
}

ValuePack Computer::getArchitecture(const ValuePack& args)
{
    return ValuePack();
}

ValuePack Computer::getArchitectures(const ValuePack& args)
{
    return ValuePack();
}

ValuePack Computer::beep(const ValuePack& args)
{
    std::cout << "\a";
    return ValuePack();
}

ValuePack Computer::getDeviceInfo(const ValuePack& args)
{
    return ValuePack();
}

ValuePack Computer::getProgramLocations(const ValuePack& args)
{
    return ValuePack();
}

// in seconds
ValuePack Computer::uptime(const ValuePack& args)
{
    return ValuePack({now() - _start_time});
}

ValuePack Computer::pushSignal(const ValuePack& args)
{
    return ValuePack();
}

ValuePack Computer::removeUser(const ValuePack& args)
{
    return ValuePack();
}

ValuePack Computer::addUser(const ValuePack& args)
{
    return ValuePack();
}

ValuePack Computer::isRobot(const ValuePack& args)
{
    return ValuePack();
}

ValuePack Computer::tmpAddress(const ValuePack& args)
{
    return ValuePack { _tmp_address };
}

ValuePack Computer::freeMemory(const ValuePack& args)
{
    return ValuePack();
}

ValuePack Computer::totalMemory(const ValuePack& args)
{
    return ValuePack();
}

ValuePack Computer::energy(const ValuePack& args)
{
    return ValuePack();
}

ValuePack Computer::maxEnergy(const ValuePack& args)
{
    return ValuePack();
}

