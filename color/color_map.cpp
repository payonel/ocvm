#include "color_map.h"
#include <tuple>
#include <limits>
using namespace std;

int ColorMap::monochrome_color = 0x00af00;

enum CShift
{
    RShift32 = 16,
    GShift32 =  8,
    BShift32 =  0
};

enum CBits
{
    Reds   = 6,
    Greens = 8,
    Blues  = 5
};

class ColorFormat
{
public:
    virtual int inflate(int value) const = 0;
    virtual int deflate(const Color& color) const = 0;

    static tuple<int, int, int> extract(int rgb)
    {
        int r = (rgb >> CShift::RShift32) & 0xff;
        int g = (rgb >> CShift::GShift32) & 0xff;
        int b = (rgb >> CShift::BShift32) & 0xff;
        return make_tuple(r, g, b);
    }
};


class ColorFormat_1 : public ColorFormat
{
public:
    int inflate(int value) const
    {
        if (value == 0)
            return 0;
        else
            return ColorMap::monochrome_color;
    }

    int deflate(const Color& color) const override
    {
        return color.rgb == 0 ? 0 : 1;
    }
} cf_1;

class ColorFormat_4 : public ColorFormat
{
public:
    int inflate(int value) const override
    {
        int index = std::max((size_t)0, std::min(PALETTE_SIZE, (size_t)value));
        return palette[index];
    }

    static inline constexpr bool isFromPalette(int value)
    {
        return value >= 0 && value <= (int)PALETTE_SIZE;
    }

    static inline constexpr int delta(int a, int b)
    {
        int rA = (a >> CShift::RShift32) & 0xff;
        int gA = (a >> CShift::GShift32) & 0xff;
        int bA = (a >> CShift::BShift32) & 0xff;

        int rB = (b >> CShift::RShift32) & 0xff;
        int gB = (b >> CShift::GShift32) & 0xff;
        int bB = (b >> CShift::BShift32) & 0xff;

        int dr = rA - rB;
        int dg = gA - gB;
        int db = bA - bB;

        return 0.2126 * dr * dr + 0.7152 * dg * dg + 0.0722 * db * db;
    }

    int deflate(const Color& color) const override
    {
        if (color.paletted)
        {
            return std::max(0, color.rgb) % PALETTE_SIZE;
        }

        // search for palette entry with smallest distance from color
        int min_distance = std::numeric_limits<int>::max();
        size_t best_index = 0;

        for (size_t i = 0; i < PALETTE_SIZE && min_distance > 0; i++)
        {
            const int& palette_entry = palette[i];
            int distance = delta(palette_entry, color.rgb);
            if (distance < min_distance)
            {
                min_distance = distance;
                best_index = i;
            }
        }

        return best_index;
    }

protected:
    int palette[PALETTE_SIZE] {
        0xFFFFFF, 0xFFCC33, 0xCC66CC, 0x6699FF,
        0xFFFF33, 0x33CC33, 0xFF6699, 0x333333,
        0xCCCCCC, 0x336699, 0x9933CC, 0x333399,
        0x663300, 0x336600, 0xFF3333, 0x000000
    };
} cf_4;

class ColorFormat_8 : public ColorFormat_4
{
public:

    ColorFormat_8()
    {
        // Initialize palette to grayscale, excluding black and white, because
        // those are already contained in the normal color cube.
        for (size_t i = 0; i < PALETTE_SIZE; i++)
        {
            int shade = 0xff * (i + 1) / (PALETTE_SIZE + 1);
            palette[i] = 
                (shade << CShift::RShift32) | 
                (shade << CShift::GShift32) | 
                (shade << CShift::BShift32);
        }
    }

    int inflate(int value) const override
    {
        if (isFromPalette(value))
        {
            return ColorFormat_4::inflate(value);
        }
        int index = value - (int)PALETTE_SIZE;
        int idxB = index % CBits::Blues;
        int idxG = (index / CBits::Blues) % CBits::Greens;
        int idxR = (index / CBits::Blues / CBits::Greens) % CBits::Reds;
        int r = (int)(idxR * 0xFF / (CBits::Reds - 1.0) + 0.5);
        int g = (int)(idxG * 0xFF / (CBits::Greens - 1.0) + 0.5);
        int b = (int)(idxB * 0xFF / (CBits::Blues - 1.0) + 0.5);
        return
            (r << RShift32) |
            (g << GShift32) |
            (b << BShift32);
    }

    int deflate(const Color& color) const override
    {
        int paletteIndex = ColorFormat_4::deflate(color) & 0xFF;
        if (color.paletted)
        {
            return paletteIndex;
        }

        auto rgb = extract(color.rgb);
        int idxR = (int)(std::get<0>(rgb) * (CBits::Reds - 1.0) / 0xFF + 0.5);
        int idxG = (int)(std::get<1>(rgb) * (CBits::Greens - 1.0) / 0xFF + 0.5);
        int idxB = (int)(std::get<2>(rgb) * (CBits::Blues - 1.0) / 0xFF + 0.5);
        int deflated = (PALETTE_SIZE + idxR * CBits::Greens * CBits::Blues + idxG * CBits::Blues + idxB) & 0xFF;
        if (delta(inflate(deflated), color.rgb) < delta(inflate(paletteIndex), color.rgb))
        {
            return deflated;
        }
        
        return paletteIndex;
    }
} cf_8;

const ColorFormat* toFormater(EDepthType depth)
{
    const ColorFormat* pf = nullptr;
    switch (depth)
    {
        case EDepthType::_1: pf = &cf_1; break;
        case EDepthType::_4: pf = &cf_4; break;
        case EDepthType::_8: pf = &cf_8; break;
    }
    return pf;
}

int ColorMap::inflate(int value, EDepthType depth)
{
    const ColorFormat* pf = toFormater(depth);
    return pf->inflate(value);
}

int ColorMap::deflate(const Color& color, EDepthType depth)
{
    const ColorFormat* pf = toFormater(depth);
    return pf->deflate(color);
}

void ColorMap::redeflate(Color* pColor, EDepthType old_depth, EDepthType new_depth)
{
    pColor->rgb = inflate(pColor->rgb, old_depth);
    pColor->rgb = deflate(*pColor, new_depth);
}
