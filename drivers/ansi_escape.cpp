#include "ansi_escape.h"
#include "model/log.h"

#include <iostream>
using std::cout;
using std::flush;

#include <unistd.h>
#include <sys/ioctl.h>
#include <signal.h>

#include "ansi.h"
#include "apis/unicode.h"
#include "raw_tty.h"

tuple<int, int> current_resolution()
{
    int cols = 80;
    int lines = 24;

#ifdef TIOCGSIZE
    struct ttysize ts;
    ioctl(STDOUT_FILENO, TIOCGSIZE, &ts);
    cols = ts.ts_cols;
    lines = ts.ts_lines;
#elif defined(TIOCGWINSZ)
    struct winsize ts;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ts);
    cols = ts.ws_col;
    lines = ts.ws_row;
#endif /* TIOCGSIZE */

    return std::make_tuple(cols, lines);
}

static sigset_t g_sigset;

AnsiEscapeTerm::AnsiEscapeTerm()
{
    sigemptyset(&g_sigset);
    sigaddset(&g_sigset, SIGWINCH);

    if (pthread_sigmask(SIG_BLOCK, &g_sigset, nullptr))
    {
        lout << "failed to mask threads for winch - resize will be ignored\n";
        return;
    }
}

AnsiEscapeTerm::~AnsiEscapeTerm()
{
    pthread_sigmask(SIG_UNBLOCK, &g_sigset, nullptr);
    cout << Ansi::clear_term << Ansi::set_pos(1, 1) << flush;
}

void AnsiEscapeTerm::onUpdate()
{
    timespec timeout { 0, 0 }; // poll, do not block
    if (sigtimedwait(&g_sigset, nullptr, &timeout) == SIGWINCH) // winched
    {
        cout << Ansi::clear_scroll << flush;
        auto rez = current_resolution();
        int width = std::get<0>(rez);
        int height = std::get<1>(rez);
        winched(width, height);
    }
    cout << flush;
}

tuple<int, int> AnsiEscapeTerm::onOpen()
{
    //switch to alternative buffer screen
    // cout << esc << "47h";

    cout << Ansi::cursor_off;
    clear();

    TtyReader::engine()->start(this);

    return current_resolution();
}

void AnsiEscapeTerm::onClose()
{
    TtyReader::engine()->stop();
    cout << Ansi::cursor_on;
    cout << Ansi::set_pos(1, 1);
    cout << flush;
}

static string replace_all(const string& src, const string& match, const string& replacement)
{
    size_t index = 0;
    string result = "";
    while (index < src.size())
    {
        size_t next = src.find(match, index);
        result += src.substr(index, next - index);
        index = next;
        if (next != string::npos)
        {
            result += replacement;
            index += match.size() + 1;
        }
        else
        {
            // in case of signed size_t index is not >= src.size() here
            break;
        }
    }
    return result;
}

string AnsiEscapeTerm::scrub(const string& value) const
{
    // replace tabs with (U+2409 for HT symbol)
    // I could use the ht unicode symbol in the source file
    // but i prefer to keep the source files in ascii
    return replace_all(value, "\t", string{(char)226, (char)144, (char)137, ' '});
}

void AnsiEscapeTerm::onWrite(int x, int y, const Cell& cell)
{
    string cmd = "";
    if (x != _x || y != _y)
    {
        if (x == 1 && y == _y + 1) // new line
        {
            cmd += "\r\n";
        }
        else
        {
            cmd += Ansi::set_pos(x, y);
        }
    }
    if (cell.fg.rgb != _fg_rgb || cell.bg.rgb != _bg_rgb)
        cmd += Ansi::set_color(cell.fg, cell.bg);

    string text = scrub(cell.value);
    cout << cmd << text;
    _x = x + cell.width;
    _y = y;
    _fg_rgb = cell.fg.rgb;
    _bg_rgb = cell.bg.rgb;

    // if _x or _y are outside the window, flush
    auto rez = size();
    if (_x > std::get<0>(rez) || _y > std::get<1>(rez))
        cout << flush;
}

void AnsiEscapeTerm::onClear()
{
    cout << Ansi::clear_term << Ansi::set_pos(1, 1) << flush;
}
