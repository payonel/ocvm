#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <tuple>
#include <memory>

using std::tuple;
using std::vector;
using std::string;
using std::unique_ptr;

#include "color/color_types.h"
#include "io/event.h"

struct Cell
{
    string value; // must be a string for multibyte chars
    Color fg;
    Color bg;
    bool locked; // locked when a double wide is placed before it
    int width;
};

class Screen;

class Frame
{
protected:
    // REQUIRED PURE VIRTUALS
    // this is called when the emulator wants you to render text in your window
    virtual void onWrite(int x, int y, const Cell& cell) = 0;

    // It is required to return the initial size of the window from onOpen
    // If the user later changes the size of the window ...
    // (e.g. dragging the border with a mouse)
    // then you need to call winched with the new resolution
    virtual tuple<int, int> onOpen() = 0;

    // The following methods are likely helpful to derived types
public:
    // call close when this window would like to abort the vm
    //  e.g. the user tries to close the window frame
    void close();

    // Call mouseEvent when you want to push mouse events to the vm
    // see io/event.h for MouseEvent details
    void mouseEvent(const MouseEvent& me);

    // Call keyEvent when you want to push key events to the vm
    // see io/event.h for KeyEvent details
    void keyEvent(const KeyEvent& ke);

    // it is optional to override depth
    virtual EDepthType depth() { return EDepthType::_8; }

protected:
    // You need to call winched if the user is resizing the Frame window manually
    // The VM does not call winched, you do (e.g. from onUpdate())
    // this is reported to component.gpu.maxResolution()
    void winched(int width, int height);

    // it is optional to override these methods as you need them
    virtual void onUpdate() {}
    virtual void onClose() {}
    virtual void onClear() {}

///////////////////////////////////////////////////////////////////////////////////////////
    // The remaining methods should only be called by the emulator
public:
    virtual ~Frame();

    // called by Screen when initializing
    void open(Screen* screen);

    // called by Screen during vm loop
    bool update();

    // called by the gpu
    void write(int x, int y, const Cell& cell);
    void clear();

    // size is updated by calling winched
    tuple<int, int> size() const;

    // the screen component uses the frame for turning on and off
    bool on() const;
    bool on(bool bOn);
private:
    Screen* _screen = nullptr;

    int _width;
    int _height;
    bool _isOn;
};

namespace Factory
{
    // Add your frame type to create_frame
    // see drivers/factory_shell.cpp
    Frame* create_frame(const string& frameTypeName);
};

