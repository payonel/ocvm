#pragma once

#include "io/kb_drv.h"

class KeyboardScannerPrivate;
class KeyboardScanner : public KeyboardDriver
{
public:
    KeyboardScanner();
    ~KeyboardScanner();
    void proc() override;

private:
    KeyboardScannerPrivate* _priv;
};
