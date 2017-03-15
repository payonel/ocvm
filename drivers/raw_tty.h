#pragma once

#include "mouse_drv.h"
#include "kb_drv.h"

class TermInputStream
{
public:
    virtual unsigned char get() = 0;
    virtual bool hasMouseCode() = 0;
};

class TermInputDriver : public virtual InputDriver
{
public:
    ~TermInputDriver();
    virtual void enqueue(TermInputStream* stream) = 0;

protected:
    bool onStart() override;
    void onStop() override;
};

class MouseTerminalDriver : public TermInputDriver, public MouseDriverImpl
{
public:
    void enqueue(TermInputStream* stream) override;
    static bool isAvailable();
};

class KeyboardLocalRawTtyDriver : public TermInputDriver, public KeyboardDriverImpl
{
public:
    void enqueue(TermInputStream* stream) override;
    static bool isAvailable();
};

class KeyboardPtyDriver : public TermInputDriver, public KeyboardDriverImpl
{
public:
    void enqueue(TermInputStream* stream) override;
    static bool isAvailable();
};
