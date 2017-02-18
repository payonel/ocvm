#pragma once
#include "component.h"
#include "value.h"
#include "framing/frame.h"

struct Color
{
    int rgb;
    bool paletted;
};

struct Cell
{
    string value; // must be a string for multibyte chars
    Color fg;
    Color bg;
};

class Screen : public Component, public Frame
{
public:
    Screen();
    int getKeyboards(lua_State*);
    bool setResolution(int width, int height, bool bQuiet = false) override;

    const Cell* get(int x, int y) const;
    void set(int x, int y, const Cell& cell);
    void set(int x, int y, const string& text);
    void set(int x, int y, const vector<const Cell*>& scanned);
    vector<const Cell*> scan(int x, int y, int width) const;

    void foreground(const Color& color);
    const Color& foreground() const;
    void background(const Color& color);
    const Color& background() const;
protected:
    bool onInitialize(Value& config) override;
    void resizeBuffer(int width, int height);
private:
    vector<string> _keyboards;
    Cell* _cells = nullptr;
    Color _bg;
    Color _fg;
};
