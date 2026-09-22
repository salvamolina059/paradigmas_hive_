#include "AwaitingThrowDestination.h"

#include <utility>

#include "ActionMenu.h"
#include "AwaitingVictim.h"
#include "Idle.h"
#include "hive/Game.h"

AwaitingThrowDestination::AwaitingThrowDestination(std::vector<PillbugThrow> candidates,
                                                     std::optional<PieceType> imitating)
    : candidates_(std::move(candidates)), imitating_(imitating)
{
}

std::unique_ptr<InputState> AwaitingThrowDestination::handleBoardClick(Game &game, const Hex &hex)
{
    for (const PillbugThrow &t : candidates_)
    {
        if (t.destination == hex)
        {
            game.applyMove(t);
            return std::make_unique<Idle>();
        }
    }
    return nullptr;
}

std::unique_ptr<InputState> AwaitingThrowDestination::cancel(Game &game)
{
    ActionMenu menu(game, candidates_.front().pillbug, imitating_);
    return menu.chooseAction(game, ActionKind::Throw);
}

std::optional<Hex> AwaitingThrowDestination::selectedHex() const
{
    return candidates_.front().pillbug;
}

std::vector<Hex> AwaitingThrowDestination::highlightedHexes() const
{
    std::vector<Hex> hexes;
    for (const PillbugThrow &t : candidates_)
    {
        hexes.push_back(t.destination);
    }
    return hexes;
}

std::optional<PieceType> AwaitingThrowDestination::currentImitation() const
{
    return imitating_;
}

bool AwaitingThrowDestination::requiresExplicitCancel() const
{
    return true;
}
