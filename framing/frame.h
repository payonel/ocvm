#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <tuple>

using std::tuple;
using std::vector;
using std::string;

class Frame;

class Framer
{
public:
    Framer();
    virtual ~Framer();
    bool add(Frame* pf, size_t index = static_cast<size_t>(-1));

    virtual bool open() = 0;
    virtual void close() = 0;
    virtual bool update() = 0;
    virtual void onWrite(Frame*) = 0;
    virtual void onResolution(Frame*) = 0;
    virtual tuple<int, int> maxResolution() const = 0;
protected:
    virtual bool onAdd(Frame* pf) = 0;
    vector<Frame*> _frames;
};

class Frame
{
public:
    Frame();
    virtual ~Frame();
    void framer(Framer* pfr);
    Framer* framer() const;

    virtual void mouse(int x, int y) {}
    virtual void keyboard(char c) {}

    virtual void write(const string& text);
    virtual void move(int x, int y) {}
    bool empty() const;
    tuple<int, int, string> pop();

    bool setResolution(int width, int height, bool bQuiet = false);
    tuple<int, int> getResolution() const;
    bool scrolling() const;
    void scrolling(bool enable);

    int x() const;
    int y() const;
private:
    Framer* _framer;
    vector<tuple<int, int, string>> _buffer;

    int _width;
    int _height;

    int _x;
    int _y; // not used if scrolling

    bool _scrolling;
};

