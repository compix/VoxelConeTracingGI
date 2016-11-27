#pragma once
#include <cstdint>

struct RGB;
struct HSV;

struct RGB8
{
    uint8_t r{ 0 };
    uint8_t g{ 0 };
    uint8_t b{ 0 };

    RGB8() { }

    RGB8(uint8_t r, uint8_t g, uint8_t b)
        : r(r), g(g), b(b) { }
};

struct RGBA8
{
    uint8_t r{0};
    uint8_t g{0};
    uint8_t b{0};
    uint8_t a{0};

    RGBA8() { }

    RGBA8(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
        : r(r), g(g), b(b), a(a) { }
};

struct RGBA32F
{
    float r{0};
    float g{0};
    float b{0};
    float a{0};

    RGBA32F() { }

    RGBA32F(float r, float g, float b, float a)
        : r(r), g(g), b(b), a(a) { }
};

struct RGBA_UINT32
{
    uint32_t r{0};
    uint32_t g{0};
    uint32_t b{0};
    uint32_t a{0};

    RGBA_UINT32() { }

    RGBA_UINT32(uint32_t r, uint32_t g, uint32_t b, uint32_t a)
        : r(r), g(g), b(b), a(a) { }
};

struct RG_UINT32
{
    uint32_t r{0};
    uint32_t g{0};

    RG_UINT32() { }

    RG_UINT32(uint32_t r, uint32_t g)
        : r(r), g(g) { }
};

struct RGB
{
    RGB() { }

    /**
    * @brief	Values in [0, 255]
    */
    RGB(float r, float g, float b) : r(r), g(g), b(b) { }

    float r{0.f};
    float g{0.f};
    float b{0.f};

    static const RGB Red;
    static const RGB Green;
    static const RGB Blue;
    static const RGB Yellow;

    HSV toHSV() const;
};

struct HSV
{
    HSV() { }

    /**
    * @brief	Hue in [0, 360]�. s, v in [0, 100]%
    */
    HSV(float h, float s, float v) : h(h), s(s), v(v) { }

    operator RGB() const { return toRGB(); }

    float h; // angle in degrees
    float s; // percent
    float v; // percent

    RGB toRGB() const;
};
