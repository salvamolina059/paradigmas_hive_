#include "InputState.h"

std::unique_ptr<InputState> InputState::handleBoardClick(Game &, const Hex &)
{
    return nullptr;
}

std::unique_ptr<InputState> InputState::handleHandClick(Game &, Color, PieceType)
{
    return nullptr;
}

std::unique_ptr<InputState> InputState::chooseAction(Game &, ActionKind)
{
    return nullptr;
}

std::unique_ptr<InputState> InputState::chooseImitation(Game &, PieceType)
{
    return nullptr;
}

std::unique_ptr<InputState> InputState::cancel(Game &)
{
    return nullptr;
}

std::optional<Hex> InputState::selectedHex() const
{
    return std::nullopt;
}

std::optional<PieceType> InputState::selectedHandPieceType() const
{
    return std::nullopt;
}

std::vector<Hex> InputState::highlightedHexes() const
{
    return {};
}

std::vector<ActionKind> InputState::actionOptions() const
{
    return {};
}

std::vector<PieceType> InputState::imitationOptions() const
{
    return {};
}

std::optional<PieceType> InputState::currentImitation() const
{
    return std::nullopt;
}

bool InputState::requiresExplicitCancel() const
{
    return false;
}
