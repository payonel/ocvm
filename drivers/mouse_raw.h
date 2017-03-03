#pragma once

#include "io/mouse_drv.h"

struct MouseLocalRawTtyDriverPrivate;
class MouseLocalRawTtyDriver : public MouseDriver
{
public:
    MouseLocalRawTtyDriver();
    ~MouseLocalRawTtyDriver();

protected:
    void onStart() override;
    bool runOnce() override;
    void onStop() override;

private:
    MouseLocalRawTtyDriverPrivate* _priv;
};
