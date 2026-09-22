#include "MoveFields.h"

#include <variant>

std::optional<Hex> sourceHex(const Move &move)
{
    if (const auto *movement = std::get_if<Movement>(&move))
    {
        return movement->from;
    }
    if (const auto *mosquito = std::get_if<MosquitoMovement>(&move))
    {
        return mosquito->from;
    }
    if (const auto *throwMove = std::get_if<PillbugThrow>(&move))
    {
        return throwMove->pillbug;
    }
    return std::nullopt; // Placement
}

Hex destinationHex(const Move &move)
{
    return std::visit([](const auto &m) { return m.destination; }, move);
}
