#pragma once

#include "component.h"

class Modem : public Component
{
public:
    Modem();

    enum ConfigIndex
    {
        SystemPort = Component::ConfigIndex::Next,
    };
protected:
    bool onInitialize() override;
};
