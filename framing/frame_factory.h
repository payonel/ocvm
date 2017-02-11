#pragma once

class Framer;

class FrameFactory
{
public:
    enum EFramer
    {
        Basic,
        Curses
    };
    static Framer* create(EFramer eFramerType);
};
