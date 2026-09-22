#pragma once

#include <SFML/System/Vector2.hpp>

#include "hive/Hex.h"

// Axial (q, r) <-> pixel conversion for a flat-top hex grid (redblobgames
// formulas: https://www.redblobgames.com/grids/hexagons/). SFML-only file
// -- must not be included by anything in core -- but the only SFML
// dependency is sf::Vector2f as the pixel-space value type, shared with
// the rest of gui/; it's a header-only template, so this doesn't pull
// -lsfml-system into anything that links it (see tests/hex_layout_test.cpp).
class HexLayout
{
public:
    // `size` is the center-to-vertex radius in pixels; `origin` is the
    // pixel position of Hex{0, 0}.
    HexLayout(float size, sf::Vector2f origin);

    sf::Vector2f hexToPixel(const Hex &hex) const;

    // The center-to-vertex radius this layout was built with -- callers
    // that computed a fitted size (see GameView's auto-fit) need it back
    // to size tiles/sprites consistently with hexToPixel's spacing.
    float size() const;

    // Inverse of hexToPixel: the hex whose tile contains pixel point `p`.
    // Uses cube-coordinate rounding (round q, r, and the implied cube `s
    // = -q-r` independently, then recompute whichever rounded furthest
    // from its fractional value) -- plain truncation of the fractional
    // axial coordinates below picks the wrong hex for points near an edge.
    Hex pixelToHex(sf::Vector2f p) const;

private:
    float size_;
    sf::Vector2f origin_;
};
