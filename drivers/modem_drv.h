#pragma once

#include <string>
#include <vector>
using std::string;
using std::vector;

#include "io/event.h"

class ModemDriver
{
public:
    ModemDriver(EventSource<ModemEvent>* source);
    bool close(int port);
    bool open(int port);
    bool isOpen(int port);
    bool broadcast(int port, const vector<char>& payload);
    bool send(string address, int port, const vector<char>& payload);

    bool start();
    void stop();
private:
    EventSource<ModemEvent>* _source = nullptr;
};
