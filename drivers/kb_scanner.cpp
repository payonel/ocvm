#include "kb_scanner.h"

#include <stdio.h>
#include <ctype.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <bitset>
#include <iostream>
using namespace std;

static inline unsigned char bit(char* keys, int index)
{
    return ((keys[index/8]) & (1<<(index%8)));
}

class KeyboardScannerPrivate
{
public:
    void open_display()
    {
        _display = XOpenDisplay(nullptr);
        _window_stack.clear();
        _modifiers.clear();

        XModifierKeymap* mapping = XGetModifierMapping(_display);

        int width = mapping->max_keypermod;
        auto& map = mapping->modifiermap;
        for (int mod_index = ShiftMapIndex; mod_index <= Mod5MapIndex; mod_index++)
        {
            for (int i = 0; i < width; i++)
            {
                uint code = map[mod_index*width + i];
                if (code)
                {
                    _modifiers[code] = make_tuple(mod_index, i);
                }
            }
        }

        XFreeModifiermap(mapping);

        _key_template = XKeyEvent();
        _key_template.display = _display;
        _key_template.time = CurrentTime;
        _key_template.send_event = true;
        _key_template.same_screen = true;
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

        _key_template.window = w;
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

    bool infer_next_event(XEvent* pev)
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

            pev->xkey = _key_template;

            bool pressed = (current & first_bit);
            pev->type = pressed ? KeyPress : KeyRelease;

            // cout << "computed code: " << i << ' ' << (i*8) << ' ' << hex << (int)first_bit << ' ' << dec << code << endl;
            const auto& modifier_set_iterator = _modifiers.find(code);
            if (modifier_set_iterator != _modifiers.end())
            {
                const auto& mod_key_tuple = modifier_set_iterator->second;

                uint mod_index = std::get<0>(mod_key_tuple); // shift(0), lock(1), ctrl(2), etc
                uint nth_code = std::get<1>(mod_key_tuple); // the nth code in the group

                bitset<8> mod_bits = _mod_groups[mod_index];
                mod_bits.set(nth_code, pressed);
                _mod_groups[mod_index] = static_cast<unsigned char>(mod_bits.to_ulong());

                bitset<8> state_bits = _modifier_state;
                state_bits.set(mod_index, mod_bits.any());
                _modifier_state = static_cast<unsigned char>(state_bits.to_ulong());
            }
            pev->xkey.state = _modifier_state;
            pev->xkey.keycode = code;
            return true;
        }

        return false;
    }

    bool run_once(XEvent* pev)
    {
        if (is_in_focus() && infer_next_event(pev))
        {
            return true;
        }

        return false;
    }

private:
    unordered_set<Window> _window_stack;
    unordered_map<uint, tuple<uint, uint>> _modifiers;
    unsigned char _mod_groups[8] {}; // 8 mod keys, 8 possible locations of those keys

    Display* _display {};
    Window _top {};
    char _keymap[32] {};
    unsigned char _modifier_state {};
    XKeyEvent _key_template;
};

KeyboardScanner::KeyboardScanner() :
    _priv(new KeyboardScannerPrivate)
{
}

KeyboardScanner::~KeyboardScanner()
{
    delete _priv;
    _priv = nullptr;
}

void KeyboardScanner::proc()
{
    _priv->open_display();
    _priv->goto_parent();
    _priv->monitor_tree();
    char buf[32] {};

    while (_continue)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        XEvent event {};
        if (_priv->run_once(&event))
        {
            if (event.type != KeyPress && event.type != KeyRelease)
            {
                continue;
            }

            buf[0] = 0;
            KeySym ks;
            int len = XLookupString(&event.xkey, buf, sizeof(buf) - 1, &ks, nullptr);
            buf[len] = 0;

            enqueue(event.type == KeyPress, buf, ks, len, event.xkey.keycode, event.xkey.state);
        }
    }

    _priv->close_display();
}
