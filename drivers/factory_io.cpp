#include "kb_scanner.h"
#include "raw_tty.h"

#include <memory>
#include <vector>
#include <string>
#include <iostream>
using namespace std;

unique_ptr<KeyboardDriver> Factory::create_kb(const string& kbTypeName)
{   
    if (kbTypeName.empty())
    {
        // requesting raw kb explicitly should always succeed because
        // raw kb driver use file io
        if (KeyboardLocalRawTtyDriver::isAvailable())
            return Factory::create_kb("raw");

        vector<string> defaults { "scanner", "pty" };
        for (auto name : defaults)
        {
            auto result = Factory::create_kb(name);
            if (result)
            {
                cout << name << " kb created\r\n";
                return result;
            }
        }
    }
    else if (kbTypeName == "raw")
    {
        return unique_ptr<KeyboardDriver>(new KeyboardLocalRawTtyDriver);
    }
    else if (kbTypeName == "scanner")
    {
        if (KeyboardScanner::isAvailable())
            return unique_ptr<KeyboardDriver>(new KeyboardScanner);
    }
    else if (kbTypeName == "pty")
    {
        if (KeyboardPtyDriver::isAvailable())
            return unique_ptr<KeyboardDriver>(new KeyboardPtyDriver);
    }

    return nullptr;
}

unique_ptr<MouseDriver> Factory::create_mouse(const string& mouseTypeName)
{
    if (mouseTypeName.empty() || mouseTypeName == "raw")
    {
        return unique_ptr<MouseDriver>(new MouseTerminalDriver);
    }

    return nullptr;
}
