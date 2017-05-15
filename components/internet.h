#pragma once
#include "component.h"
#include "model/value.h"

class Internet : public Component
{
public:
    enum ConfigIndex
    {
        // ScreenAddress = Component::ConfigIndex::Next
    };

    Internet();
    ~Internet();
    // bool postInit() override;
    // RunState update() override;
protected:
    bool onInitialize() override;
private:
};
