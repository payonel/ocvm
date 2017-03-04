#pragma once

#include "io/mouse_input.h"

class MouseDriverImpl : public MouseDriver
{
public:
    void enqueue();
};
