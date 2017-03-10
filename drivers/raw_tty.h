#pragma once

#include "mouse_drv.h"
#include "kb_drv.h"

class TermInputStream
{
public:
    virtual unsigned char get() = 0;
};

class TermInputDriver
{
public:
    virtual void enqueue(TermInputStream* stream) = 0;
};

class MouseTerminalDriver : public TermInputDriver, public MouseDriverImpl
{
public:
    ~MouseTerminalDriver();

    void enqueue(TermInputStream* stream) override;
    static bool isAvailable();

protected:
    bool onStart() override;
    void onStop() override;
};

class KeyboardLocalRawTtyDriver : public TermInputDriver, public KeyboardDriverImpl
{
public:
    ~KeyboardLocalRawTtyDriver();

    void enqueue(TermInputStream* stream) override;
    static bool isAvailable();

protected:
    bool onStart() override;
    void onStop() override;
};

class KeyboardPtyDriver : public TermInputDriver, public KeyboardDriverImpl
{
public:
    ~KeyboardPtyDriver();

    void enqueue(TermInputStream* stream) override;
    static bool isAvailable();

protected:
    bool onStart() override;
    void onStop() override;
};
