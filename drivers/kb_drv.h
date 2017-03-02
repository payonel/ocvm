#pragma once

#include <mutex>
#include <unordered_map>
#include <string>
#include <queue>
#include <thread>
using std::mutex;
using std::unordered_map;
using std::string;
using std::queue;
using std::thread;

struct KeyEvent
{
    string text;
    unsigned int keysym;
    unsigned int keycode;
    bool bPressed; // false: released

    bool bShift; //0x1
    bool bControl; // 0x4
    bool bAlt; // 0x8
};

class KeyboardDriver
{
public:
    KeyboardDriver();
    virtual ~KeyboardDriver();
    bool isRunning();

    bool start();
    void stop();
    bool pop(KeyEvent* pke);

protected:
    uint map_code(const uint& code);
    uint map_sym(const uint& sym, int sequence_length);
    virtual void proc() = 0;
    // TODO enqueue should take just K_RAW stdin data
    // void enqueue(char* bytes, uint length);
    void enqueue(bool bPressed, const string& text, uint keysym, uint sequence_length, uint keycode, uint state);
    volatile bool _continue {};
private:
    unordered_map<uint, uint> _codes;
    mutex _m;
    queue<KeyEvent> _events;
    thread* _pthread {};
    bool _running {};
};
