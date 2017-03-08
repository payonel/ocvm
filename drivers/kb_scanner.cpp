#include "kb_scanner.h"
#include "worker.h"

#include <stdio.h>
#include <ctype.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <unordered_map>
#include <unordered_set>
#include <bitset>
#include <iostream>
using namespace std;

static inline unsigned char bit(char* keys, int index)
{
    return ((keys[index/8]) & (1<<(index%8)));
}

class KeyboardScannerPrivate : public Worker
{
public:
    KeyboardScannerPrivate(KeyboardScanner* driver) :
        _driver(driver)
    {}

    void open_display()
    {
        _display = XOpenDisplay(nullptr);
        _window_stack.clear();
    }

    void close_display()
    {
        XCloseDisplay(_display);
        _window_stack.clear();
    }

    void goto_parent()
    {
        int revert_to;
        Window w;
        XGetInputFocus(_display, &w, &revert_to);
        while (true)
        {
            Window root;
            Window parent;
            Window* children = nullptr;
            uint numChildren = 0;
            XQueryTree(_display, w, &root, &parent, &children, &numChildren);
            XFree(children);

            if (parent == root)
            {
                break;
            }

            w = parent;
        }

        _top = w;
    }

    void monitor_tree(Window w = None)
    {
        if (w == None)
        {
            w = _top;
        }
        _window_stack.insert(w);

        Window _r, _p;
        Window* children = nullptr;
        uint numChildren = 0;
        XQueryTree(_display, w, &_r, &_p, &children, &numChildren);
        for (uint child = 0; child < numChildren; child++)
        {
            monitor_tree(children[child]);
        }
        XFree(children);
    }

    bool is_in_focus()
    {
        Window w;
        int revert;
        XGetInputFocus(_display, &w, &revert);
        return _window_stack.find(w) != _window_stack.end();
    }

    bool infer_next_event(uint* pCode, bool* pbPressed)
    {
        char keymap[32] {};
        XQueryKeymap(_display, keymap);
        // only scan for first change
        for (int i = 0; i < 32; i++)
        {
            // this should get optimized out
            const char& stored = _keymap[i];
            const char& current = keymap[i];
            unsigned char xored = stored ^ current;
            unsigned char first_bit = xored & (xored - 1);
            first_bit = ~first_bit & xored;
            uint code = i * 8;

            switch (first_bit)
            {
                case 0x80: code += 1;
                case 0x40: code += 1;
                case 0x20: code += 1;
                case 0x10: code += 1;
                case 0x8:  code += 1;
                case 0x4:  code += 1;
                case 0x2:  code += 1;
                case 0x1:  //code += 1;
                    break;
                default: continue;
            }

            _keymap[i] ^= first_bit;

            *pbPressed = (current & first_bit);
            *pCode = code;
            return true;
        }

        return false;
    }

    bool runOnce() override
    {
        if (!is_in_focus())
            return true;

        uint code;
        bool pressed;
        if (infer_next_event(&code, &pressed))
        {
             _driver->enqueue(pressed, code);
        }

        return true;
    }

    void onStart() override
    {
        open_display();
        goto_parent();
        monitor_tree();
    }

    void onStop() override
    {
        close_display();
    }

private:
    unordered_set<Window> _window_stack;

    Display* _display {};
    Window _top {};
    char _keymap[32] {};
    unsigned char _modifier_state {};
    KeyboardScanner* _driver;
};

KeyboardScanner::KeyboardScanner() :
    _priv(new KeyboardScannerPrivate(this))
{
}

KeyboardScanner::~KeyboardScanner()
{
    stop();
    delete _priv;
    _priv = nullptr;
}

bool KeyboardScanner::onStart()
{
    _priv->start();
    return true;
}

void KeyboardScanner::onStop()
{
    _priv->stop();
}

