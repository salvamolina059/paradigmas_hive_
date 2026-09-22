#include "Idle.h"

#include <variant>

#include "ActionMenu.h"
#include "AwaitingDestination.h"
#include "MoveFields.h"
#include "hive/Game.h"

Idle::Idle() = default;

std::unique_ptr<InputState> Idle::handleBoardClick(Game &game, const Hex &hex)
{
    for (const Move &move : game.legalMoves())
    {
        if (sourceHex(move) == hex)
        {
            return std::make_unique<ActionMenu>(game, hex);
        }
    }
    return nullptr;
}

std::unique_ptr<InputState> Idle::handleHandClick(Game &game, Color color, PieceType type)
{
    if (color != game.turn())
    {
        return nullptr;
    }

    std::vector<Move> candidates;
    for (const Move &move : game.legalMoves())
    {
        const auto *placement = std::get_if<Placement>(&move);
        if (placement != nullptr && placement->type == type)
        {
            candidates.push_back(move);
        }
    }
    if (candidates.empty())
    {
        return nullptr;
    }
    return std::make_unique<AwaitingDestination>(std::move(candidates));
}
