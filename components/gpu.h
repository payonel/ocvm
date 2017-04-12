#pragma once
#include "component.h"
#include "model/value.h"
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
    int set(int x, int y, const Cell& cell, bool bForce);
    void set(int x, int y, const vector<char>& text, bool bVertical);

    Cell* at(int x, int y) const;
    bool setResolution(int width, int height);

    bool onInitialize() override;
    void check(lua_State* lua) const; // throws if no screen

    int setColorContext(lua_State* lua, bool bBack);
    int getColorContext(lua_State* lua, bool bBack);
    tuple<int, Value> makeColorContext(const Color& color);

    void resizeBuffer(int width, int height);
    void invalidate();

    // color mapping to oc 256 codes
    Color deflate(const Color& color);
    void inflate_all();
    void deflate_all();
    unsigned char encode(int rgb);

    // FrameGpu overrides
    void winched(int width, int height) override;
private:
    Screen* _screen = nullptr;

    int _width = 0;
    int _height = 0;

    Cell* _cells = nullptr;
    Color _bg;
    Color _fg;
    ColorState _color_state;
};
