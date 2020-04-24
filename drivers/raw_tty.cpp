#include "raw_tty.h"

#include <iostream>
#include <set>
#include <sstream>

#include <stdio.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#ifndef __APPLE__
#ifdef __linux__
#include <linux/kd.h>
#endif
#include <sys/select.h>

// explicitly needed for include on Haiku OS
#include <sys/time.h>
#include <sys/types.h>
#endif

#ifdef __HAIKU__
#define KDGKBMODE 0x4B44 /* gets current keyboard mode */
#define KDSKBMODE 0x4B45 /* sets current keyboard mode */
#endif

#include <signal.h>
#include <string.h> // memset
#include <time.h>

#include "ansi.h"
#include "drivers/ansi_escape.h"

using std::cerr;
using std::cout;
using std::flush;
using std::set;

#ifndef __APPLE__
static unsigned long _original_kb_mode = 0;
#endif

static struct sigaction sig_action_data;
static termios* original_termios = nullptr;

static void exit_function()
{
#ifdef __linux__
  // leave raw mode
  ioctl(0, KDSKBMODE, _original_kb_mode);
#endif
  if (original_termios)
  {
    ::tcsetattr(STDIN_FILENO, TCSANOW, original_termios);
  }
  cout << Ansi::mouse_prd_off;
  cout << Ansi::cursor_on;
  cout << Ansi::color_reset << flush;
  delete original_termios;
  original_termios = nullptr;
}

static void segfault_sigaction(int signal, siginfo_t* pSigInfo, void* arg)
{
  exit_function();
  cout << "\nocvm caught a SIGSEGV signal. " << flush;
  std::abort();
}

void TtyReader::start(AnsiEscapeTerm* pTerm)
{
  _pTerm = pTerm;
  _kb_drv = KeyboardTerminalDriver::create(hasMasterTty());

  if (hasTerminalOut())
  {
    cout << Ansi::mouse_prd_on << flush;
    _mouse_drv.reset(new MouseTerminalDriver);
  }

  Worker::start();
}

// static
TtyReader* TtyReader::engine()
{
  static TtyReader one;
  static bool init = false;
  if (!init)
  {
    init = true;
    memset(&sig_action_data, 0, sizeof(sig_action_data));
    sig_action_data.sa_sigaction = segfault_sigaction;
    sig_action_data.sa_flags = SA_SIGINFO;
    sigaction(SIGSEGV, &sig_action_data, nullptr);
  }
  return &one;
}

bool TtyReader::hasMasterTty() const
{
  return _master_tty;
}

bool TtyReader::hasTerminalOut() const
{
  return _terminal_out;
}

TtyReader::TtyReader()
{
  _master_tty = false;
  _terminal_out = false;

#ifndef __APPLE__
  int ec = 0;
  ec = ioctl(0, KDGKBMODE, &_original_kb_mode);
  if (ec == 0) // success
  {
    ec = ioctl(0, KDSKBMODE, _original_kb_mode);
  }
  _master_tty = ec == 0 && errno == 0;
#endif

  _terminal_out = (isatty(fileno(stdout)));
}

bool TtyReader::onStart()
{
  //save current settings
  original_termios = new termios;
  ::tcgetattr(STDIN_FILENO, original_termios);

  //put in raw mod
  termios raw;
  memset(&raw, 0, sizeof(termios));
  ::cfmakeraw(&raw);
  ::tcsetattr(STDIN_FILENO, TCSANOW, &raw);

#ifdef __linux__
  if (_master_tty)
  {
    int ec = ioctl(0, KDGKBMODE, &_original_kb_mode);
    ec = ioctl(0, KDSKBMODE, K_RAW);
    if (ec != 0 || errno != 0)
    {
      // try to reset kb JUST IN CASE
      exit_function();
      cerr << "\ncritical failure: could not set raw mode\n";
      std::abort();
    }

    atexit(&exit_function);
  }
#endif

  cout << flush;
  return true;
}

// void log_codes(TermBuffer* buffer)
// {
//     auto size = buffer->size();
//     std::cerr << "{";
//     for (decltype(buffer->size()) i = 0; i < size; i++)
//     {
//         std::cerr << static_cast<int>(buffer->peek(i));
//         if (i < size - 1)
//             std::cerr << ",";
//     }
//     std::cerr << "}\n";
// }

bool TtyReader::runOnce()
{
  while (true)
  {
    char byte = 0;
    struct timeval tv
    {
      0L, 0L
    };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    if (select(1, &fds, nullptr, nullptr, &tv))
    {
      if (read(0, &byte, sizeof(char)) > 0)
      {
        _buffer.push(byte);
        continue;
      }
    }

    break;
  }

  auto old_size = _buffer.size();
  if (old_size > 0)
  {
    // log_codes(&_buffer);
    if (_mouse_drv && _pTerm)
    {
      auto vme = _mouse_drv->parse(&_buffer);
      for (const auto& me : vme)
        _pTerm->mouseEvent(me);
    }
    if (_kb_drv && _pTerm)
    {
      auto vke = _kb_drv->parse(&_buffer);
      for (const auto& ke : vke)
        _pTerm->keyEvent(ke);
    }

    if (old_size == _buffer.size()) // nothing could read the buffer
    {
      if (_buffer.hasMouseCode())
      {
        _buffer.get();
        _buffer.get();
        _buffer.get();
        _buffer.get();
        _buffer.get();
      }
      _buffer.get(); // pop one off
    }
  }
  else
  {
    if (_kb_drv && _pTerm)
    {
      auto vke = _kb_drv->idle();
      for (const auto& ke : vke)
        _pTerm->keyEvent(ke);
    }
  }

  return true;
}

void TtyReader::onStop()
{
  _pTerm = nullptr;
  exit_function();
}
