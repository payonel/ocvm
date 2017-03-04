#pragma once

#include "kb_drv.h"

class KeyboardScannerPrivate;
class KeyboardScanner : public KeyboardDriverImpl
{
public:
    KeyboardScanner();
    ~KeyboardScanner();

protected:
    bool onStart() override;
    void onStop() override;

private:
    KeyboardScannerPrivate* _priv;
};
