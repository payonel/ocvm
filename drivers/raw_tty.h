#pragma once

#include "mouse_drv.h"
#include "kb_drv.h"

class MouseLocalRawTtyDriver : public MouseDriverImpl
{
public:
    MouseLocalRawTtyDriver();
    ~MouseLocalRawTtyDriver();

protected:
    bool onStart() override;
    void onStop() override;
};

class KeyboardLocalRawTtyDriver : public KeyboardDriverImpl
{
public:
    KeyboardLocalRawTtyDriver();
    ~KeyboardLocalRawTtyDriver();

    static bool isAvailable();

protected:
    bool onStart() override;
    void onStop() override;
};
