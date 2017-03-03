#include "kb_scanner.h"
#include "mouse_raw.h"

KeyboardDriver* Factory::create_kb(const string& kbTypeName)
{
    if (kbTypeName == "scanner")
    {
        return new KeyboardScanner;
    }

    return nullptr;
}

MouseDriver* Factory::create_mouse(const string& mouseTypeName)
{
    if (mouseTypeName == "raw")
    {
        return new MouseLocalRawTtyDriver;
    }

    return nullptr;
}
