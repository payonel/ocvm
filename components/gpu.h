#pragma once
#include "component.h"
#include "value.h"
#include <tuple>
using std::tuple;

class Screen;

class Gpu : public Component
{
public:
    Gpu();

    ValuePack setResolution(const ValuePack& args);
    ValuePack bind(const ValuePack& args);
    ValuePack set(const ValuePack& args);
    ValuePack maxResolution(const ValuePack& args);
    ValuePack setBackground(const ValuePack& args);
    ValuePack getBackground(const ValuePack& args);
    ValuePack setForeground(const ValuePack& args);
    ValuePack getForeground(const ValuePack& args);
    ValuePack fill(const ValuePack& args);
    ValuePack copy(const ValuePack& args);
protected:
    bool onInitialize(Value& config) override;
    bool truncateWH(int x, int y, int* pWidth, int* pHeight) const;
private:
    Screen* _screen;
};
