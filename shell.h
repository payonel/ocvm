#pragma once

class Client;

class Shell
{
public:
    Shell(Client*);
    bool update();
    void close();
};
