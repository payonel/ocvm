#include "kb_scanner.h"
#include "mouse_raw.h"

#include <memory>
using std::unique_ptr;

unique_ptr<KeyboardDriver> Factory::create_kb(const string& kbTypeName)
{
    if (kbTypeName == "scanner")
    {
        return unique_ptr<KeyboardDriver>(new KeyboardScanner);
    }

    return nullptr;
}

unique_ptr<MouseDriver> Factory::create_mouse(const string& mouseTypeName)
{
    if (mouseTypeName == "raw")
    {
        return unique_ptr<MouseDriver>(new MouseLocalRawTtyDriver);
    }

    return nullptr;
}
