#pragma once
#include "component.h"
#include "io/event.h"
#include "io/frame.h"
#include "model/value.h"

class Keyboard;
class Gpu;

class Screen : public Component, public IScreen, private EventSource<MouseEvent>
{
public:
  Screen();
  ~Screen();
  RunState update() override;

  // Component API
  int isOn(lua_State*);
  int getAspectRatio(lua_State*);
  int getKeyboards(lua_State*);
  int setTouchModeInverted(lua_State*);
  int turnOn(lua_State*);
  int turnOff(lua_State*);
  int isPrecise(lua_State*);
  int isTouchInverted(lua_State*);
  int setPrecise(lua_State*);

  bool connectKeyboard(Keyboard* kb);
  bool disconnectKeyboard(Keyboard* kb);
  vector<string> keyboards() const;

  void push(const KeyEvent& ke) override;
  void push(const MouseEvent& ke) override;
  bool setResolution(int width, int height) override;
  void invalidate() override;

  void gpu(Gpu* gpu);
  Gpu* gpu() const;
  Frame* frame() const;

protected:
  bool onInitialize() override;
  
  Value getDeviceInfo() const override;

private:
  vector<Keyboard*> _keyboards;
  unique_ptr<Frame> _frame;
  Gpu* _gpu = nullptr;

  static bool s_registered;
};
