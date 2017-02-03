#pragma once
#include <string>
#include <iostream>

class Logger
{
};

extern Logger log;
Logger& operator<< (Logger&, const std::string& text);
