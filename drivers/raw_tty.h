#pragma once

#include "mouse_drv.h"
#include "kb_drv.h"
#include "term_buffer.h"
#include "worker.h"

#include <memory>
using std::unique_ptr;

struct termios;
class Framer;

class TtyReader : public Worker
{
public:
    TtyReader(TtyReader&) = delete;
    void operator= (TtyReader&) = delete;

    static TtyReader* engine();
    void start(Framer* pFramer);
    void stop();

private:
    bool hasMasterTty() const;
    bool hasTerminalOut() const;
    TtyReader();
    void onStart() override;
    bool runOnce() override;
    void onStop() override;

    bool _master_tty;
    bool _terminal_out;
    termios* _original = nullptr;
    TermBuffer _buffer;

    unique_ptr<MouseTerminalDriver> _mouse_drv;
    unique_ptr<KeyboardTerminalDriver> _kb_drv;

    Framer* _pFramer = nullptr;
};
