#include "AwaitingDestination.h"

#include <utility>
#include <variant>

#include "ActionMenu.h"
#include "Idle.h"
#include "MoveFields.h"
#include "hive/Game.h"

AwaitingDestination::AwaitingDestination(std::vector<Move> candidates, std::optional<PieceType> imitating)
    : candidates_(std::move(candidates)), imitating_(imitating)
{
}

std::unique_ptr<InputState> AwaitingDestination::handleBoardClick(Game &game, const Hex &hex)
{
    for (const Move &move : candidates_)
    {
        if (destinationHex(move) == hex)
        {
            game.applyMove(move);
            return std::make_unique<Idle>();
        }
    }
    return cancel(game);
}

std::unique_ptr<InputState> AwaitingDestination::handleHandClick(Game &game, Color color, PieceType type)
{
    Idle idle;
    return idle.handleHandClick(game, color, type);
}

std::unique_ptr<InputState> AwaitingDestination::cancel(Game &game)
{
    const std::optional<Hex> source = sourceHex(candidates_.front());
    if (!source.has_value())
    {
        return std::make_unique<Idle>();
    }
    return std::make_unique<ActionMenu>(game, *source, imitating_);
}

std::optional<Hex> AwaitingDestination::selectedHex() const
{
    return sourceHex(candidates_.front());
}

std::optional<PieceType> AwaitingDestination::selectedHandPieceType() const
{
    if (const auto *placement = std::get_if<Placement>(&candidates_.front()))
    {
        return placement->type;
    }
    return std::nullopt;
}

std::vector<Hex> AwaitingDestination::highlightedHexes() const
{
    std::vector<Hex> hexes;
    for (const Move &move : candidates_)
    {
        hexes.push_back(destinationHex(move));
    }
    return hexes;
}

std::optional<PieceType> AwaitingDestination::currentImitation() const
{
    return imitating_;
}
