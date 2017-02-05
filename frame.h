#pragma once

#include <functional>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>

class Frame;

class Framer
{
public:
    Framer();
    virtual ~Framer();
    virtual bool add(Frame* pf);

    virtual bool open() = 0;
    virtual void close() = 0;
    virtual bool update() = 0;
    virtual void onWrite(Frame*) = 0;
    virtual void onResolution(Frame*, int oldw, int oldh) = 0;
protected:
    std::vector<Frame*> _frames;
};

class Frame
{
public:
    Frame();
    virtual ~Frame();
    void setFramer(Framer* pfr);

    virtual void mouse(int x, int y) {}
    virtual void keyboard(char c) {}

    void write(const std::string& text);
    std::string read();

    virtual bool setResolution(int width, int height);
    void getResolution(int* pWidth ,int* pHeight);
private:
    Framer* _framer;
    std::string _buffer;

    int _width;
    int _height;
};

Frame& operator<< (Frame&, const std::string& text);
Frame& operator<< (Frame&, std::ostream& (*)(std::ostream&));

template <typename T>
Frame& operator<< (Frame& frame, const T& t)
{
    std::stringstream ss;
    ss << t;
    frame << ss.str();
    return frame;
}

