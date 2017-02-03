#pragma once
#include "iframe.h"
#include <string>
#include <iostream>

class Logger : public IFrame
{
};

extern Logger log;
Logger& operator<< (Logger&, const std::string& text);
