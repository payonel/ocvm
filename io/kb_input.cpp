#include "kb_input.h"

bool KeyboardInput::open(unique_ptr<KeyboardDriver> driver)
{
    KeyboardDriver* pkb = driver.release();
    unique_ptr<InputDriver> pid(pkb);
    return InputSource::open(std::move(pid));
}

unique_ptr<KeyEvent> KeyboardInput::pop()
{
    return unique_ptr<KeyEvent>(static_cast<KeyEvent*>(InputSource::pop().release()));
}
