#include "screen.h"

#include <iostream>
#include <vector>

#include "client.h"
#include "host.h"
#include "framing/frame.h"

Screen::Screen()
{
    add("getKeyboards", &Screen::getKeyboards);
}

bool Screen::onInitialize(Value& config)
{
    // we now have a client and can add ourselves to the framer
    return client()->host()->getFramer()->add(this, 0);
}

ValuePack Screen::getKeyboards(const ValuePack& args)
{
    ValuePack pack({Value::table()});
    Value& list = pack.at(0);
    for (const auto& kb : _keyboards)
    {
        list.insert(kb);
    }
    return pack;
}
