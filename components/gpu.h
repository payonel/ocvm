#pragma once
#include "component.h"
#include "value.h"
#include "color/color_types.h"
#include "io/frame.h"
#include <tuple>
#include <vector>
using std::tuple;
using std::vector;

enum class EDepthType;
class Screen;

class Gpu : public Component, public FrameGpu
{
public:
    Gpu();
    ~Gpu();

    enum ConfigIndex
    {
        Palette = Component::ConfigIndex::Next,
        MonochromeColor
    };

    int getResolution(lua_State* lua);
    int setResolution(lua_State* lua);
    int bind(lua_State* lua);
    int set(lua_State* lua);
    int get(lua_State* lua);
    int maxResolution(lua_State* lua);
    int setBackground(lua_State* lua);
    int getBackground(lua_State* lua);
    int setForeground(lua_State* lua);
    int getForeground(lua_State* lua);
    int fill(lua_State* lua);
    int copy(lua_State* lua);
    int getDepth(lua_State* lua);
    int setDepth(lua_State* lua);
    int getViewport(lua_State* lua);
    int setViewport(lua_State* lua);
    int getScreen(lua_State* lua);
    int maxDepth(lua_State* lua);
    int getPaletteColor(lua_State* lua);
    int setPaletteColor(lua_State* lua);
protected:
    vector<const Cell*> scan(int x, int y, int width) const;
    const Cell* get(int x, int y) const;
    void set(int x, int y, const Cell& cell);
    void set(int x, int y, const string& text);
    void set(int x, int y, const vector<const Cell*>& scanned);

    void setResolution(int width, int height);

    bool onInitialize() override;
    void check(lua_State* lua) const; // throws if no screen
    void deflate(lua_State* lua, Color* pRawColor);

    int setColorContext(lua_State* lua, bool bBack);
    int getColorContext(lua_State* lua, bool bBack);
    tuple<int, Value> makeColorContext(const Color& color);

    EDepthType setDepth(EDepthType depth);
    EDepthType getDepth() const override;

    void resizeBuffer(int width, int height);
    void invalidate();

    // FrameGpu overrides
    void winched(int width, int height) override;
private:
    Screen* _screen = nullptr;

    int _width;
    int _height;

    Cell* _cells = nullptr;
    Color _bg;
    Color _fg;
    EDepthType _depth;
};
