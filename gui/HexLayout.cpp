#include "HexLayout.h"

#include <cmath>

HexLayout::HexLayout(float size, sf::Vector2f origin) : size_(size), origin_(origin)
{
}

float HexLayout::size() const
{
    return size_;
}

sf::Vector2f HexLayout::hexToPixel(const Hex &hex) const
{
    const float q = static_cast<float>(hex.q);
    const float r = static_cast<float>(hex.r);
    const float x = size_ * 1.5f * q;
    const float y = size_ * (std::sqrt(3.f) * 0.5f * q + std::sqrt(3.f) * r);
    return {origin_.x + x, origin_.y + y};
}

Hex HexLayout::pixelToHex(sf::Vector2f p) const
{
    const float x = (p.x - origin_.x) / size_;
    const float y = (p.y - origin_.y) / size_;

    const float qf = (2.f / 3.f) * x;
    const float rf = (-1.f / 3.f) * x + (std::sqrt(3.f) / 3.f) * y;
    const float cubeS = -qf - rf; // cube coordinate s = -q-r

    float q = std::round(qf);
    float r = std::round(rf);
    float s = std::round(cubeS);

    const float qDiff = std::abs(q - qf);
    const float rDiff = std::abs(r - rf);
    const float sDiff = std::abs(s - cubeS);

    if (qDiff > rDiff && qDiff > sDiff)
    {
        q = -r - s;
    }
    else if (rDiff > sDiff)
    {
        r = -q - s;
    }
    // else: s rounded furthest, but s isn't part of an axial Hex --
    // dropping it is exactly the fix-up, no further assignment needed.

    return Hex{static_cast<int>(q), static_cast<int>(r)};
}
