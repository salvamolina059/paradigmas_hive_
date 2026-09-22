#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "hive/Enums.h"
#include "hive/Hex.h"
#include "input/InputState.h"

class Game;

// Owns the InputState state machine (Idle, ActionMenu,
// AwaitingDestination, AwaitingVictim, AwaitingThrowDestination -- see
// InputState.h) and is the only thing main.cpp/GameView talk to;
// neither ever sees an InputState directly.
//
// Every mutator forwards to state_ and, if a non-null unique_ptr comes
// back, transitions to it; a null return means "no change," so
// state_ is left as-is. Every query is a plain read-only forward, used
// by GameView to decide what to highlight/draw -- it never makes its
// own legality decisions, since state_'s candidates always trace back
// to a real Game::legalMoves() snapshot.
class InputController
{
public:
    explicit InputController(Game &game);

    // hex is the result of HexLayout::pixelToHex on a board click;
    // color/type identify which hand-panel piece was clicked.
    void handleBoardClick(const Hex &hex);
    void handleHandClick(Color color, PieceType type);

    // The two circular menu buttons and the mosquito imitation-target
    // icons, per current actionOptions()/imitationOptions().
    void chooseAction(ActionKind kind);
    void chooseImitation(PieceType type);

    // The explicit Cancel button.
    void cancel();

    std::optional<Hex> selectedHex() const;
    std::optional<PieceType> selectedHandPieceType() const;
    std::vector<Hex> highlightedHexes() const;
    std::vector<ActionKind> actionOptions() const;
    std::vector<PieceType> imitationOptions() const;
    std::optional<PieceType> currentImitation() const;
    bool requiresExplicitCancel() const;

private:
    Game &game_;
    std::unique_ptr<InputState> state_;
};
