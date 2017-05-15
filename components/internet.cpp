#include "internet.h"
#include "model/log.h"

Internet::Internet()
{
}

Internet::~Internet()
{
}

bool Internet::onInitialize()
{
    // _preferredScreen = config().get(ConfigIndex::ScreenAddress).Or("").toString();
    return true;
}

/*
bool Internet::postInit()
{
    return true;
}

RunState Internet::update()
{
    return RunState::Continue;
}
*/
