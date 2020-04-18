#include "kb_drv.h"
#include "drivers/kb_data.h"
#include "term_buffer.h"

#include <bitset>
#include <chrono>
#include <set>

static KBData kb_data;

class KeyboardTerminalDriverCommon : public KeyboardTerminalDriver
{
public:
  KeyboardTerminalDriverCommon()
  {
    _modifier_state = 0;
  }

protected:
  void mark(bool bPressed, _Code keycode, vector<KeyEvent>* pOut)
  {
    // filter out some events
    switch (keycode)
    {
    case 42 | 0x80: // PRINT SCREEN (comes in a pair of double bytes, 42,55 -- each are pressed and unpressed)
    case 46 | 0x80: // FN+F6 (SPEAKER VOLUME DOWN) (double byte)
    case 48 | 0x80: // FN+F7 (SPEAKER VOLUME UP) (double byte)
    case 55 | 0x80: // PRINT SCREEN (comes in a pair of double bytes, 42,55 -- each are pressed and unpressed)
    case 76 | 0x80: // FN+F9 (DISPLAY BACKLIGHT DECREASE) (double byte)
    case 84 | 0x80: // FN+F10 (DISPLAY BACKLIGHT INCREASE) (double byte)
    case 86 | 0x80: // FN+F4 (DISPLAY) (double byte)
    case 95 | 0x80: // FN+F3 (SLEEP) (double byte)
      return;
    case 219: // WINDOWS
    case 221: // MENU
      keycode = 0;
      break;
    }

    KeyEvent ke;
    ke.bPressed = bPressed;
    ke.keycode = keycode;

    update_modifier(bPressed, ke.keycode);
    ke.keysym = kb_data.lookup(ke.keycode, static_cast<ModBit>(_modifier_state));

    ke.bShift = (_modifier_state & (_Mod)ModBit::Shift);
    ke.bCaps = (_modifier_state & (_Mod)ModBit::Caps);
    ke.bControl = (_modifier_state & (_Mod)ModBit::Control);
    ke.bAlt = (_modifier_state & (_Mod)ModBit::Alt);
    ke.bNumLock = (_modifier_state & (_Mod)ModBit::NumLock);

    // unusual in-game shifted keycodes
    if (_modifier_state & (_Mod)ModBit::Shift)
    {
      switch (keycode) // keycodes shift
      {
      case 3:
        ke.keycode = 145;
        break; // 2
      case 7:
        ke.keycode = 144;
        break; // 6
      case 12:
        ke.keycode = 147;
        break; // -
      case 39:
        ke.keycode = 146;
        break; // ;
      }
    }

    pOut->push_back(ke);
  }

  void update_modifier(bool bPressed, _Code keycode)
  {
    const auto& modifier_set_iterator = kb_data.modifiers.find(keycode);
    if (modifier_set_iterator != kb_data.modifiers.end())
    {
      const auto& mod_key_tuple = modifier_set_iterator->second;

      unsigned int mod_index = std::get<0>(mod_key_tuple); // shift(0), lock(1), ctrl(2), etc
      unsigned int nth_code = std::get<1>(mod_key_tuple);  // the nth code in the group

      std::bitset<8> mod_bits = _mod_groups[mod_index];
      mod_bits.set(nth_code, bPressed);
      _mod_groups[mod_index] = static_cast<unsigned char>(mod_bits.to_ulong());

      std::bitset<8> state_bits = _modifier_state;
      state_bits.set(mod_index, mod_bits.any());
      _modifier_state = static_cast<unsigned char>(state_bits.to_ulong());
    }
  }

  vector<KeyEvent> idle() override
  {
    return {};
  }

  _Mod _modifier_state;

private:
  unsigned char _mod_groups[8]{}; // 8 mod keys, 8 possible locations of those keys
};

class KeyboardLocalRawTtyDriver : public KeyboardTerminalDriverCommon
{
public:
  vector<KeyEvent> parse(TermBuffer* buffer) override
  {
    if (buffer->size() == 0 || buffer->hasMouseCode())
      return {};

    bool released;
    unsigned int keycode = buffer->get();

    switch (keycode)
    {
    case 0xE0: // double byte
      keycode = buffer->get();
      released = keycode & 0x80;
      keycode |= 0x80; // add press indicator
      break;
    case 0xE1:                 // triple byte
      keycode = buffer->get(); // 29(released) or 29+0x80[157](pressed)
      released = keycode & 0x80;
      // NUMLK is a double byte 0xE0, 69 (| x80)
      // PAUSE is a triple byte 0xE1, 29 (| x80), 69 (| 0x80)
      // because triple byte press state is encoded in the 2nd byte
      // the third byte should retain 0x80
      keycode = buffer->get() | 0x80;
      break;
    default:
      released = keycode & 0x80;
      keycode &= 0x7F; // remove pressed indicator
      break;
    }

    vector<KeyEvent> vke;
    mark(!released, keycode, &vke);
    return vke;
  }
};

class KeyboardPtyDriver : public KeyboardTerminalDriverCommon
{
  inline void wakeup(bool activity)
  {
    if (activity)
    {
      _last_idle = std::chrono::system_clock::now();
    }
  }

public:
  vector<KeyEvent> parse(TermBuffer* buffer) override
  {
    if (buffer->size() == 0 || buffer->hasMouseCode())
      return {};

    vector<KeyEvent> events;

    _Mod mod;
    _Code code;
    if (!kb_data.lookup(buffer, &code, &mod))
    {
      if (buffer->size()) // insert?
      {
        KeyEvent ke;
        while (buffer->size())
          ke.insert.push_back(buffer->get());
        events.push_back(ke);
      }
      wakeup(!events.empty());
      return events;
    }

    if (mod != _modifier_state)
    {
      // send key release
      // ~new (0101) & old (1001) = 0001
      _Mod released = ~mod & _modifier_state;
      auto codes = kb_data.getModCodes(released);
      for (auto modCode : codes)
        mark(false, modCode, &events);

      // send key press
      // new (1010) & ~old (0110) = 0010
      _Mod pressed = mod & ~_modifier_state;
      codes = kb_data.getModCodes(pressed);
      for (auto modCode : codes)
        mark(true, modCode, &events);

      _modifier_state = mod;
    }

    // keep history of last N pressed keys
    if (_pressedCodesCache.find(code) == _pressedCodesCache.end())
    {
      _pressedCodesCache.insert(code);
    }

    mark(true, code, &events);
    wakeup(!events.empty());
    return events;
  }

  vector<KeyEvent> idle() override
  {
    if (_pressedCodesCache.empty())
      return {};

    constexpr std::chrono::milliseconds idle_timeout{ 500 };
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    auto time_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - _last_idle);

    vector<KeyEvent> events;
    if (time_elapsed > idle_timeout)
    {
      // send key release
      // ~new (0101) & old (1001) = 0001
      _Mod released = ~0 & _modifier_state;
      auto codes = kb_data.getModCodes(released);
      for (auto modCode : codes)
        mark(false, modCode, &events);

      _modifier_state = 0;

      // mark all keys as released
      for (const auto& code : _pressedCodesCache)
      {
        mark(false, code, &events);
      }
      _pressedCodesCache.clear();
    }

    return events;
  }

private:
  std::set<_Code> _pressedCodesCache;
  std::chrono::system_clock::time_point _last_idle = std::chrono::system_clock::now();
};

// static
std::unique_ptr<KeyboardTerminalDriver> KeyboardTerminalDriver::create(bool bMaster)
{
  if (bMaster)
    return std::make_unique<KeyboardLocalRawTtyDriver>();
  else
    return std::make_unique<KeyboardPtyDriver>();
}
