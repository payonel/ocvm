#pragma once
#include "component.h"
#include "model/value.h"
#include "io/frame.h"

class MouseInput;
class Keyboard;
class Gpu;

class Screen : public Component
{
public:
    Screen();
    ~Screen();
    RunState update() override;

    int getKeyboards(lua_State*);
    int getAspectRatio(lua_State*);

    bool connectKeyboard(Keyboard* kb);
    bool disconnectKeyboard(Keyboard* kb);
    vector<string> keyboards() const;

    void push(const KeyEvent& ke);
    void push(const MouseEvent& me);
    void gpu(Gpu* gpu);
    Gpu* gpu() const;
    Frame* frame() const;
protected:
    bool onInitialize() override;
private:
    vector<Keyboard*> _keyboards;
    unique_ptr<MouseInput> _mouse;
    unique_ptr<Frame> _frame;
    Gpu* _gpu;
};
