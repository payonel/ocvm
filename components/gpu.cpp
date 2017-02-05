#include "gpu.h"
#include "log.h"
#include <iostream>

Gpu::Gpu(const ValuePack& args) :
    Component("gpu")
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
    log << "set resolution: " << args.at(0).serialize() << ", " << args.at(1).serialize() << std::endl;
    return ValuePack();
}
