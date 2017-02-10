#include "gpu.h"
#include "log.h"
#include "frame.h"
#include <iostream>

Gpu::Gpu(const string& type, const Value& init, Host* host) :
    Component(type, init, host)
{
    add("setResolution", &Gpu::setResolution);
    add("bind", &Gpu::bind);
    add("set", &Gpu::set);
}

ValuePack Gpu::bind(const ValuePack& args)
{
    lout << "BIND!" << endl;
    return ValuePack();
}

ValuePack Gpu::setResolution(const ValuePack& args)
{
    lout << "set resolution: " << args.at(0).serialize() << ", " << args.at(1).serialize() << endl;
    return ValuePack();
}

ValuePack Gpu::set(const ValuePack& args)
{
    int x = args.at(0).toNumber();
    int y = args.at(1).toNumber();
    string text = args.at(2).toString();

    // get the bound ScreenFrame and write to it
    // how??

    return ValuePack();
}
