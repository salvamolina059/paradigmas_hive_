#include "hive/Hex.h"

#include <cmath>
#include <cstdint>

bool operator==(const Hex &a, const Hex &b)
{
    return a.q == b.q && a.r == b.r;
}

const std::array<Hex, 6> &hexDirections()
{
    static const std::array<Hex, 6> directions = {
        Hex{1, 0}, Hex{1, -1}, Hex{0, -1},
        Hex{-1, 0}, Hex{-1, 1}, Hex{0, 1},
    };
    return directions;
}

std::vector<Hex> neighbors(const Hex &h)
{
    std::vector<Hex> result;
    for (const Hex &dir : hexDirections())
    {
        result.push_back(Hex{h.q + dir.q, h.r + dir.r});
    }
    return result;
}

int distance(const Hex &a, const Hex &b)
{
    const int dq = a.q - b.q;
    const int dr = a.r - b.r;
    return (std::abs(dq) + std::abs(dr) + std::abs(dq + dr)) / 2;
}

std::size_t std::hash<Hex>::operator()(const Hex &h) const
{
    // Para cualquier tablero realista, q y r entran en 32 bits cada uno, así que
    // empaquetarlos en un solo valor de 64 bits no pierde nada: dos pares (q, r)
    // distintos nunca chocan.
    const std::uint64_t packed =
        (static_cast<std::uint64_t>(static_cast<std::uint32_t>(h.q)) << 32) |
        static_cast<std::uint32_t>(h.r);
    return std::hash<std::uint64_t>{}(packed);
}
