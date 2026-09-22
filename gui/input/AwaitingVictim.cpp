#include "AwaitingVictim.h"

#include <utility>

#include "ActionMenu.h"
#include "AwaitingThrowDestination.h"

AwaitingVictim::AwaitingVictim(std::vector<PillbugThrow> candidates, std::optional<PieceType> imitating)
    : candidates_(std::move(candidates)), imitating_(imitating)
{
}

std::unique_ptr<InputState> AwaitingVictim::handleBoardClick(Game &, const Hex &hex)
{
    std::vector<PillbugThrow> destinations;
    for (const PillbugThrow &t : candidates_)
    {
        if (t.victim == hex)
        {
            destinations.push_back(t);
        }
    }
    if (destinations.empty())
    {
        return nullptr;
    }
    return std::make_unique<AwaitingThrowDestination>(std::move(destinations), imitating_);
}

std::unique_ptr<InputState> AwaitingVictim::cancel(Game &game)
{
    return std::make_unique<ActionMenu>(game, candidates_.front().pillbug, imitating_);
}

std::optional<Hex> AwaitingVictim::selectedHex() const
{
    return candidates_.front().pillbug;
}

std::vector<Hex> AwaitingVictim::highlightedHexes() const
{
    std::vector<Hex> hexes;
    for (const PillbugThrow &t : candidates_)
    {
        hexes.push_back(t.victim);
    }
    return hexes;
}

std::optional<PieceType> AwaitingVictim::currentImitation() const
{
    return imitating_;
}

bool AwaitingVictim::requiresExplicitCancel() const
{
    return true;
}
