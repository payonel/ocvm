#include "computer.h"
#include "log.h"

#include <iostream>
#include <chrono>
using namespace std::chrono;

Computer::Computer(Value& config, Host* host) :
    Component(config, host)
{
    _start_time = now();

    add("realTime", &Computer::realTime);
    add("setArchitecture", &Computer::setArchitecture);
    add("getArchitecture", &Computer::getArchitecture);
    add("getArchitectures", &Computer::getArchitectures);
    add("beep", &Computer::beep);
    add("getDeviceInfo", &Computer::getDeviceInfo);
    add("getProgramLocations", &Computer::getProgramLocations);
    add("uptime", &Computer::uptime);
    add("pushSignal", &Computer::pushSignal);
    add("removeUser", &Computer::removeUser);
    add("addUser", &Computer::addUser);
    add("isRobot", &Computer::isRobot);
    add("tmpAddress", &Computer::tmpAddress);
    add("freeMemory", &Computer::freeMemory);
    add("totalMemory", &Computer::totalMemory);
    add("energy", &Computer::energy);
    add("maxEnergy", &Computer::maxEnergy);
}

int64_t Computer::now() const
{
    return duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
}

void Computer::setTmpAddress(const string& addr)
{
    _tmp_address = addr;
}

ValuePack Computer::realTime(const ValuePack& args)
{
    return ValuePack{now()};
}

ValuePack Computer::setArchitecture(const ValuePack& args)
{
    luaL_error(args.state, "setArchitecture not implemented");
    return ValuePack();
}

ValuePack Computer::getArchitecture(const ValuePack& args)
{
    luaL_error(args.state, "getArchitecture not implemented");
    return ValuePack();
}

ValuePack Computer::getArchitectures(const ValuePack& args)
{
    luaL_error(args.state, "getArchitectures not implemented");
    return ValuePack();
}

ValuePack Computer::beep(const ValuePack& args)
{
    lout << "\a" << "BEEP\n";
    return ValuePack();
}

ValuePack Computer::getDeviceInfo(const ValuePack& args)
{
    luaL_error(args.state, "getDeviceInfo not implemented");
    return ValuePack();
}

ValuePack Computer::getProgramLocations(const ValuePack& args)
{
    luaL_error(args.state, "getProgramLocations not implemented");
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
    luaL_error(args.state, "removeUser not implemented");
    return ValuePack();
}

ValuePack Computer::addUser(const ValuePack& args)
{
    luaL_error(args.state, "addUser not implemented");
    return ValuePack();
}

ValuePack Computer::isRobot(const ValuePack& args)
{
    return ValuePack { false };
}

ValuePack Computer::tmpAddress(const ValuePack& args)
{
    return ValuePack { _tmp_address };
}

ValuePack Computer::freeMemory(const ValuePack& args)
{
    return ValuePack { std::numeric_limits<double>::max() };
}

ValuePack Computer::totalMemory(const ValuePack& args)
{
    luaL_error(args.state, "totalMemory not implemented");
    return ValuePack();
}

ValuePack Computer::energy(const ValuePack& args)
{
    luaL_error(args.state, "energy not implemented");
    return ValuePack();
}

ValuePack Computer::maxEnergy(const ValuePack& args)
{
    luaL_error(args.state, "maxEnergy not implemented");
    return ValuePack();
}

