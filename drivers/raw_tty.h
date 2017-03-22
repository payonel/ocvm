#pragma once

#include "mouse_drv.h"
#include "kb_drv.h"
#include "term_buffer.h"

class TermInputDriver : public virtual InputDriver
{
public:
    ~TermInputDriver();
    virtual void enqueue(TermBuffer* buffer) = 0;

protected:
    bool onStart() override;
    void onStop() override;
};

class MouseTerminalDriver : public TermInputDriver, public MouseDriverImpl
{
public:
    void enqueue(TermBuffer* buffer) override;
    static bool isAvailable();
};

class KeyboardLocalRawTtyDriver : public TermInputDriver, public KeyboardDriverImpl
{
public:
    void enqueue(TermBuffer* buffer) override;
    static bool isAvailable();
};

class KeyboardPtyDriver : public TermInputDriver, public KeyboardDriverImpl
{
public:
    void enqueue(TermBuffer* buffer) override;
    static bool isAvailable();
};
