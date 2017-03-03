#pragma once

#include "io/mouse_drv.h"

class MouseLocalRawTtyDriverPrivate;
class MouseLocalRawTtyDriver : public MouseDriver
{
public:
    MouseLocalRawTtyDriver();
    ~MouseLocalRawTtyDriver();

protected:
    void proc() override;
    
private:
    MouseLocalRawTtyDriverPrivate* _priv;
};
