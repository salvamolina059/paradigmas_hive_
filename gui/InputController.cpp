#include "InputController.h"

#include "input/Idle.h"

InputController::InputController(Game &game) : game_(game), state_(std::make_unique<Idle>())
{
}

void InputController::handleBoardClick(const Hex &hex)
{
    std::unique_ptr<InputState> next = state_->handleBoardClick(game_, hex);
    if (next)
    {
        state_ = std::move(next);
    }
}

void InputController::handleHandClick(Color color, PieceType type)
{
    std::unique_ptr<InputState> next = state_->handleHandClick(game_, color, type);
    if (next)
    {
        state_ = std::move(next);
    }
}

void InputController::chooseAction(ActionKind kind)
{
    std::unique_ptr<InputState> next = state_->chooseAction(game_, kind);
    if (next)
    {
        state_ = std::move(next);
    }
}

void InputController::chooseImitation(PieceType type)
{
    std::unique_ptr<InputState> next = state_->chooseImitation(game_, type);
    if (next)
    {
        state_ = std::move(next);
    }
}

void InputController::cancel()
{
    std::unique_ptr<InputState> next = state_->cancel(game_);
    if (next)
    {
        state_ = std::move(next);
    }
}

std::optional<Hex> InputController::selectedHex() const
{
    return state_->selectedHex();
}

std::optional<PieceType> InputController::selectedHandPieceType() const
{
    return state_->selectedHandPieceType();
}

std::vector<Hex> InputController::highlightedHexes() const
{
    return state_->highlightedHexes();
}

std::vector<ActionKind> InputController::actionOptions() const
{
    return state_->actionOptions();
}

std::vector<PieceType> InputController::imitationOptions() const
{
    return state_->imitationOptions();
}

std::optional<PieceType> InputController::currentImitation() const
{
    return state_->currentImitation();
}

bool InputController::requiresExplicitCancel() const
{
    return state_->requiresExplicitCancel();
}
