#include "gpu.h"
#include "log.h"
#include <iostream>

Gpu::Gpu(const std::string& type, const Value& init) :
    Component(type, init)
{
    add("setResolution", &Gpu::setResolution);
    add("bind", &Gpu::bind);
}

ValuePack Gpu::bind(const ValuePack& args)
{
    return ValuePack();
}

ValuePack Gpu::setResolution(const ValuePack& args)
{
    lout << "set resolution: " << args.at(0).serialize() << ", " << args.at(1).serialize() << std::endl;
    return ValuePack();
}
