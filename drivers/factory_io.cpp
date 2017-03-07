#include "kb_scanner.h"
#include "raw_tty.h"

#include <memory>
#include <vector>
#include <string>
using namespace std;

unique_ptr<KeyboardDriver> Factory::create_kb(const string& kbTypeName)
{   
    if (kbTypeName.empty())
    {
        vector<string> defaults { "raw", "scanner", "x", "stdin" };
        for (auto name : defaults)
        {
            auto result = Factory::create_kb(name);
            if (result)
                return result;
        }
    }
    else if (kbTypeName == "raw")
    {
        if (KeyboardLocalRawTtyDriver::isAvailable())
            return unique_ptr<KeyboardDriver>(new KeyboardLocalRawTtyDriver);
    }
    else if (kbTypeName == "scanner")
    {
        return unique_ptr<KeyboardDriver>(new KeyboardScanner);
    }

    return nullptr;
}

unique_ptr<MouseDriver> Factory::create_mouse(const string& mouseTypeName)
{
    if (mouseTypeName.empty())
    {
        vector<string> defaults { "raw", "x", "stdin" };
        for (auto name : defaults)
        {
            auto result = Factory::create_mouse(name);
            if (result)
                return result;
        }
    }
    else if (mouseTypeName == "raw")
    {
        if (MouseLocalRawTtyDriver::isAvailable())
            return unique_ptr<MouseDriver>(new MouseLocalRawTtyDriver);
    }

    return nullptr;
}
