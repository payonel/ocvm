#pragma once

#include "mouse_drv.h"

struct MouseWorkEvent : public MouseEvent
{
};

class MouseLocalRawTtyDriverPrivate;
class MouseLocalRawTtyDriver : public MouseDriverImpl
{
public:
    MouseLocalRawTtyDriver();
    ~MouseLocalRawTtyDriver();

protected:
    bool onStart() override;
    void onStop() override;

private:
    MouseLocalRawTtyDriverPrivate* _priv;
};
