#pragma once

#include "mouse_drv.h"
#include "kb_drv.h"
#include "term_buffer.h"

class TtyResponder
{
public:
    ~TtyResponder();
    virtual void push(unique_ptr<MouseEvent> pme) = 0;
    virtual void push(unique_ptr<KeyboardEvent> pke) = 0;
};

class TtyReader : public Worker
{
public:
    TtyReader(TtyReader&) = delete;
    void operator= (TtyReader&) = delete;

    void start(TtyResponder* responder);
    static TtyReader* engine();
    bool hasMasterTty() const;
    bool hasTerminalOut() const;

private:
    TtyReader()
    void flush_stdin();
    void onStart() override;
    bool runOnce() override;
    void onStop() override;

    bool _master_tty;
    bool _terminal_out;
    termios* _original = nullptr;
    TtyResponder* _responder;
    TermBuffer _buffer;

    unique_ptr<MouseTerminalDriver> _mouse_drv;
    unique_ptr<KeyboardTerminalDriver> _kb_drv;
};
