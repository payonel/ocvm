#include "gpu.h"
#include "log.h"
#include "client.h"
#include "screen.h"
#include <iostream>

Gpu::Gpu()
{
    add("setResolution", &Gpu::setResolution);
    add("bind", &Gpu::bind);
    add("set", &Gpu::set);
    add("maxResolution", &Gpu::maxResolution);
    add("getBackground", &Gpu::getBackground);
    add("setBackground", &Gpu::setBackground);
    add("getForeground", &Gpu::getForeground);
    add("setForeground", &Gpu::setForeground);
    add("fill", &Gpu::fill);
    add("copy", &Gpu::copy);
}

bool Gpu::onInitialize(Value& config)
{
    return true;
}

ValuePack Gpu::bind(const ValuePack& args)
{
    string address = Value::check(args, 0, "string").toString();
    Component* pc = client()->component(address);
    if (!pc)
    {
        return ValuePack { Value::nil, "invalid address" };
    }
    Screen* screen = dynamic_cast<Screen*>(pc);
    if (!screen)
    {
        return ValuePack { Value::nil, "not a screen" };
    }
    _screen = screen;

    return ValuePack();
}

ValuePack Gpu::setResolution(const ValuePack& args)
{
    int width = (int)Value::check(args, 0, "number").toNumber();
    int height = (int)Value::check(args, 1, "number").toNumber();

    tuple<int, int> res = _screen->getResolution();
    if (width < 1 || width > std::get<0>(res) ||
        height < 1 || height > std::get<1>(res))
    {
        luaL_error(args.state, "unsupported resolution");
        return ValuePack { };
    }

    _screen->setResolution(width, height);
    return ValuePack { true };
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

ValuePack Gpu::maxResolution(const ValuePack& args)
{
    tuple<int, int> res = _screen->getResolution();
    return ValuePack({std::get<0>(res), std::get<1>(res)});
}

ValuePack Gpu::setBackground(const ValuePack& args)
{
    lout << "TODO, stub gpu method\n";
    return ValuePack({0, false});
}

ValuePack Gpu::getBackground(const ValuePack& args)
{
    lout << "TODO, stub gpu method\n";
    return ValuePack({0, false});
}

ValuePack Gpu::setForeground(const ValuePack& args)
{
    lout << "TODO, stub gpu method\n";
    return ValuePack({0, false});
}

ValuePack Gpu::getForeground(const ValuePack& args)
{
    lout << "TODO, stub gpu method\n";
    return ValuePack({0, false});
}

ValuePack Gpu::fill(const ValuePack& args)
{
    lout << "TODO, stub gpu method\n";
    return ValuePack();
}

ValuePack Gpu::copy(const ValuePack& args)
{
    lout << "TODO, stub gpu method\n";
    return ValuePack();
}
