#pragma once

#include "InputState.h"

// Nothing selected -- the only state with no data of its own, and the
// one every other state's flow eventually leads back to, either by
// completing a move or by cancelling all the way out.
class Idle : public InputState
{
public:
    Idle();

    // Selects `hex` if it's occupied by the current player's own,
    // not-lastMoved piece and has at least one Movement,
    // MosquitoMovement, or PillbugThrow entry in game.legalMoves() --
    // transitions to a fresh ActionMenu. No-op (stays Idle) otherwise,
    // e.g. an empty hex, the opponent's piece, or a piece with no
    // legal moves this turn.
    std::unique_ptr<InputState> handleBoardClick(Game &game, const Hex &hex) override;

    // Selects `type` from `color`'s hand if color == game.turn() and
    // at least one Placement{type, *} is in game.legalMoves() --
    // transitions straight to AwaitingDestination, skipping ActionMenu:
    // placement is never ambiguous between several action kinds, so
    // there's nothing for the menu to disambiguate.
    std::unique_ptr<InputState> handleHandClick(Game &game, Color color, PieceType type) override;
};
