#pragma once

#include "mouse_drv.h"
#include "kb_drv.h"

class RawTtyInputStream
{
public:
    virtual unsigned char get() = 0;
};

class MouseLocalRawTtyDriver : public MouseDriverImpl
{
public:
    ~MouseLocalRawTtyDriver();

    void enqueue(RawTtyInputStream* stream);
    static bool isAvailable();

protected:
    bool onStart() override;
    void onStop() override;
};

class KeyboardLocalRawTtyDriver : public KeyboardDriverImpl
{
public:
    ~KeyboardLocalRawTtyDriver();

    void enqueue(RawTtyInputStream* stream);
    static bool isAvailable();

protected:
    bool onStart() override;
    void onStop() override;

private:
    void updateState(uint keycode, bool pressed);
};
