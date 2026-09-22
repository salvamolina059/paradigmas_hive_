#include "ActionMenu.h"

#include <algorithm>
#include <utility>
#include <variant>

#include "AwaitingDestination.h"
#include "AwaitingVictim.h"
#include "Idle.h"
#include "MoveFields.h"
#include "hive/Game.h"

ActionMenu::ActionMenu(const Game &game, const Hex &source, std::optional<PieceType> imitating)
    : source_(source), sourceIsMosquito_(game.board().topAt(source)->type == PieceType::Mosquito), imitating_(imitating)
{
    for (const Move &move : game.legalMoves())
    {
        if (sourceHex(move) == source_)
        {
            candidates_.push_back(move);
        }
    }
}

std::unique_ptr<InputState> ActionMenu::handleBoardClick(Game &game, const Hex &hex)
{
    if (hex == source_)
    {
        return cancel(game);
    }
    for (const Move &move : game.legalMoves())
    {
        if (sourceHex(move) == hex)
        {
            return std::make_unique<ActionMenu>(game, hex);
        }
    }
    return cancel(game);
}

std::unique_ptr<InputState> ActionMenu::handleHandClick(Game &game, Color color, PieceType type)
{
    // Try switching straight to placing the clicked hand piece first
    // (same "a click elsewhere always tries to reinterpret itself as a
    // fresh selection" rule handleBoardClick already applies above);
    // only fall back to a plain deselect if that hand click wasn't
    // actually legal (e.g. the opponent's hand, or nothing left to
    // place).
    Idle idle;
    std::unique_ptr<InputState> next = idle.handleHandClick(game, color, type);
    return next ? std::move(next) : cancel(game);
}

std::unique_ptr<InputState> ActionMenu::chooseAction(Game &, ActionKind kind)
{
    if (kind == ActionKind::Throw)
    {
        std::vector<PillbugThrow> throws;
        for (const Move &move : candidates_)
        {
            if (const auto *t = std::get_if<PillbugThrow>(&move))
            {
                throws.push_back(*t);
            }
        }
        if (throws.empty())
        {
            return nullptr;
        }
        return std::make_unique<AwaitingVictim>(std::move(throws), imitating_);
    }

    std::vector<Move> destinations;
    for (const Move &move : candidates_)
    {
        if (std::holds_alternative<Movement>(move))
        {
            destinations.push_back(move);
            continue;
        }
        const auto *mosquito = std::get_if<MosquitoMovement>(&move);
        if (mosquito != nullptr && imitating_.has_value() && mosquito->imitating == *imitating_)
        {
            destinations.push_back(move);
        }
    }
    if (destinations.empty())
    {
        return nullptr;
    }
    return std::make_unique<AwaitingDestination>(std::move(destinations), imitating_);
}

std::unique_ptr<InputState> ActionMenu::chooseImitation(Game &, PieceType type)
{
    const std::vector<PieceType> options = imitationOptions();
    if (std::find(options.begin(), options.end(), type) == options.end())
    {
        return nullptr;
    }
    imitating_ = type;
    return nullptr;
}

std::unique_ptr<InputState> ActionMenu::cancel(Game &)
{
    return std::make_unique<Idle>();
}

std::optional<Hex> ActionMenu::selectedHex() const
{
    return source_;
}

std::vector<ActionKind> ActionMenu::actionOptions() const
{
    bool hasMosquitoMovement = false;
    bool hasThrow = false;
    for (const Move &move : candidates_)
    {
        if (std::holds_alternative<MosquitoMovement>(move))
        {
            hasMosquitoMovement = true;
        }
        else if (std::holds_alternative<PillbugThrow>(move))
        {
            hasThrow = true;
        }
    }

    // A Mosquito that hasn't picked an imitation target yet has
    // nothing to move or throw as -- both options stay hidden until
    // chooseImitation() sets one (see ActionMenu.h).
    if (hasMosquitoMovement && !imitating_.has_value())
    {
        return {};
    }

    bool moveAvailable = false;
    for (const Move &move : candidates_)
    {
        if (std::holds_alternative<Movement>(move))
        {
            moveAvailable = true;
            break;
        }
        const auto *mosquito = std::get_if<MosquitoMovement>(&move);
        if (mosquito != nullptr && imitating_.has_value() && mosquito->imitating == *imitating_)
        {
            moveAvailable = true;
            break;
        }
    }

    std::vector<ActionKind> options;
    if (moveAvailable)
    {
        options.push_back(ActionKind::Move);
    }
    // Gated by imitating_ == Pillbug specifically, not merely by
    // touching one: touching a Pillbug while imitating some other type
    // still only offers that other type's Move (see Game::addMosquitoMoves).
    if (hasThrow && (!hasMosquitoMovement || imitating_ == PieceType::Pillbug))
    {
        options.push_back(ActionKind::Throw);
    }
    return options;
}

std::vector<PieceType> ActionMenu::imitationOptions() const
{
    if (!sourceIsMosquito_)
    {
        return {};
    }

    std::vector<PieceType> types;
    bool hasThrow = false;
    for (const Move &move : candidates_)
    {
        if (std::holds_alternative<PillbugThrow>(move))
        {
            hasThrow = true;
            continue;
        }
        const auto *mosquito = std::get_if<MosquitoMovement>(&move);
        if (mosquito != nullptr && std::find(types.begin(), types.end(), mosquito->imitating) == types.end())
        {
            types.push_back(mosquito->imitating);
        }
    }
    if (hasThrow && std::find(types.begin(), types.end(), PieceType::Pillbug) == types.end())
    {
        types.push_back(PieceType::Pillbug);
    }
    return types;
}

std::optional<PieceType> ActionMenu::currentImitation() const
{
    return imitating_;
}
