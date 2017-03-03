#pragma once

#include "io/kb_drv.h"

class KeyboardScannerPrivate;
class KeyboardScanner : public KeyboardDriver
{
public:
    KeyboardScanner();
    ~KeyboardScanner();

protected:
    void runOnce() override;
    void onStart() override;
    void onStop() override;

private:
    KeyboardScannerPrivate* _priv;
};
