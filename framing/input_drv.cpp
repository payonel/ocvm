#include "input_drv.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <ctype.h>

#include <queue>
#include <thread>
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

class _KeyboardDriver
{
public:
    _KeyboardDriver()
    {
        // [9, 96] = x-8
        for (uint src = 9; src <= 96; src++)
        {
            _codes[src] = src - 8;
        }
        // [133, 137] = x+86
        for (uint src = 133; src <= 137; src++)
        {
            _codes[src] = src + 86;
        }

        _codes[105] = 157;
        _codes[108] = 184;

        _codes[127] = 197;

        _codes[110] = 199;
        _codes[111] = 200;
        _codes[112] = 201;
        _codes[113] = 203;
        _codes[114] = 205;
        _codes[115] = 207;
        _codes[116] = 208;
        _codes[117] = 209;
        _codes[118] = 210;
        _codes[119] = 211;
    }

    ~_KeyboardDriver()
    {
        stop();
    }

    void stop()
    {
        _continue = false;
        if (isRunning())
        {
            _pthread->join();
        }
        _running = false;

        delete _pthread;
        _pthread = nullptr;
    }

    bool start()
    {
        if (isRunning())
            return false;

        _running = true;
        _continue = true;
        decltype(_events) empty_queue;
        std::swap(_events, empty_queue);
        _pthread = new thread(&_KeyboardDriver::proc, this);

        return true;
    }

    bool isRunning()
    {
        return _pthread && _running;
    }

    bool pop(KeyEvent* pke)
    {
        if (_events.size() == 0)
            return false;
        unique_lock<mutex> lk(_m);
        *pke = _events.front();
        _events.pop();
        return true;
    }
protected:
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

    void proc()
    {
        open_display();
        goto_parent();
        monitor_tree();
        char buf[32] {};

        while (_continue)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            XEvent event {};
            if (is_in_focus() && infer_next_event(&event))
            {
                KeyEvent ke;
                if (event.type == KeyPress)
                    ke.bPressed = true;
                else if (event.type == KeyRelease)
                {
                    ke.bPressed = false;
                }
                else
                    continue;

                buf[0] = 0;
                KeySym ks;
                int len = XLookupString(&event.xkey, buf, sizeof(buf) - 1, &ks, nullptr);

                // cout << ke.bPressed << '\t' << ks << '\t' << event.xkey.keycode;
                // cout << "\t(raw)\n";

                ke.text = buf;
                ke.keysym = map_sym(ks, len);
                ke.keycode = map_code(event.xkey.keycode);

                ke.bShift = (event.xkey.state & 0x1);
                ke.bControl = (event.xkey.state & 0x4);
                ke.bAlt = (event.xkey.state & 0x8);

                {
                    unique_lock<mutex> lk(_m);
                    _events.push(ke);
                }
            }
        }

        close_display();
    }

private:
    mutex _m;
    queue<KeyEvent> _events;
    unordered_map<uint, uint> _codes;
    unordered_set<Window> _window_stack;
    unordered_map<uint, tuple<uint, uint>> _modifiers;
    unsigned char _mod_groups[8] {}; // 8 mod keys, 8 possible locations of those keys

    thread* _pthread {};
    bool _running {};
    volatile bool _continue {};

    Display* _display {};
    Window _top {};
    char _keymap[32] {};
    unsigned char _modifier_state {};
    XKeyEvent _key_template;

    uint map_code(const uint& code)
    {
        const auto& it = _codes.find(code);
        if (it != _codes.end())
        {
            return it->second;
        }

        return code;
    }

    uint map_sym(const KeySym& sym, int sequence_length)
    {
        if (sequence_length == 0)
            return 0;

        if (sym & 0xFF00)
            return sym & 0x7F;

        return sym;
    }


} kb_drv;

namespace InputDriver
{
    void stop()
    {
        kb_drv.stop();
    }

    bool start()
    {
        if (kb_drv.isRunning())
            return false;

        return kb_drv.start();
    }

    bool pop(KeyEvent* pke)
    {
        return kb_drv.pop(pke);
    }

    bool pop(MouseEvent* pme)
    {
        return false;
    }
};

