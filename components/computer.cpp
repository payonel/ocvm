#include "computer.h"

#include <chrono>
using namespace std::chrono;

Computer::Computer(const string& type, const Value& init, Host* host) :
    Component(type, init, host)
{
    add("realTime", &Computer::realTime);
    // add("setArchitecture", &Computer::setArchitecture);
    // add("getArchitecture", &Computer::getArchitecture);
    // add("getArchitectures", &Computer::getArchitectures);
    // add("beep", &Computer::beep);
    // add("getDeviceInfo", &Computer::getDeviceInfo);
    // add("getProgramLocations", &Computer::getProgramLocations);
    // add("uptime", &Computer::uptime);
    // add("pushSignal", &Computer::pushSignal);
    // add("removeUser", &Computer::removeUser);
    // add("addUser", &Computer::addUser);
    // add("setBootAddress", &Computer::setBootAddress);
    // add("getBootAddress", &Computer::getBootAddress);
    // add("isRobot", &Computer::isRobot);
    // add("tmpAddress", &Computer::tmpAddress);
    // add("freeMemory", &Computer::freeMemory);
    // add("totalMemory", &Computer::totalMemory);
    // add("energy", &Computer::energy);
    // add("maxEnergy", &Computer::maxEnergy);
}

ValuePack Computer::realTime(const ValuePack& args)
{
    auto sec = duration_cast<seconds>(system_clock::now().time_since_epoch());
    return ValuePack{sec.count()};
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

ValuePack Computer::address(const ValuePack& args)
{
    return ValuePack();
}

ValuePack Computer::beep(const ValuePack& args)
{
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

ValuePack Computer::uptime(const ValuePack& args)
{
    return ValuePack();
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

ValuePack Computer::setBootAddress(const ValuePack& args)
{
    return ValuePack();
}

ValuePack Computer::getBootAddress(const ValuePack& args)
{
    return ValuePack();
}

ValuePack Computer::isRobot(const ValuePack& args)
{
    return ValuePack();
}

ValuePack Computer::tmpAddress(const ValuePack& args)
{
    return ValuePack();
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

