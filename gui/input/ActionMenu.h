#pragma once

#include <optional>
#include <vector>

#include "InputState.h"
#include "hive/Move.h"

// A piece is selected; the top-bar menu is showing. Always reached
// from Idle::handleBoardClick, and always shown even when there's only
// one possible action (e.g. a lone "Move" button for an Ant) --
// consistent disambiguation UI beats special-casing the common
// one-option piece.
//
// candidates_ is every Movement/MosquitoMovement/PillbugThrow sourced
// from `source_`, snapshotted once at construction -- safe, since
// nothing mutates Game while a piece is merely selected (a move is
// only ever applied from AwaitingDestination/AwaitingThrowDestination,
// at which point this instance is gone). actionOptions(),
// imitationOptions(), and chooseAction() are all just different
// groupings of this same list.
class ActionMenu : public InputState
{
public:
    // `source` must be an occupied hex with at least one matching
    // entry in game.legalMoves() -- Idle checks this before
    // constructing. `imitating` lets a state further down the flow
    // (AwaitingVictim::cancel(), AwaitingDestination::cancel()) rebuild
    // "the same ActionMenu, imitation choice and all" rather than a
    // blank one -- a fresh top-level selection never passes it.
    ActionMenu(const Game &game, const Hex &source, std::optional<PieceType> imitating = std::nullopt);

    // Re-clicking `source_` deselects (-> cancel()). Clicking a
    // different one of the current player's own selectable pieces
    // replaces the selection with a fresh ActionMenu for it. Anything
    // else deselects, same as re-clicking `source_`.
    std::unique_ptr<InputState> handleBoardClick(Game &game, const Hex &hex) override;

    // Always deselects (-> cancel()), same as clicking a board hex
    // that isn't a piece: a hand click is never part of disambiguating
    // an already-selected piece's action.
    std::unique_ptr<InputState> handleHandClick(Game &game, Color color, PieceType type) override;

    // Move -> AwaitingDestination over candidates_'s Movement/
    // MosquitoMovement entries (filtered to imitating_ for a
    // Mosquito). Throw -> AwaitingVictim over candidates_'s
    // PillbugThrow entries.
    std::unique_ptr<InputState> chooseAction(Game &game, ActionKind kind) override;

    // Ground-level Mosquito only (imitationOptions() is empty for
    // everything else): sets or switches imitating_ in place -- stays
    // in this same ActionMenu (returns nullptr), which is what makes
    // this pickable "at any time" rather than a one-shot commitment.
    std::unique_ptr<InputState> chooseImitation(Game &game, PieceType type) override;

    std::optional<Hex> selectedHex() const override;

    // {Move} for a piece with no PillbugThrow entries; {Move, Throw}
    // for a real Pillbug, or a Mosquito currently imitating one (see
    // Game::addMosquitoMoves) -- i.e. gated by imitating_ ==
    // PieceType::Pillbug, not merely by a Pillbug being adjacent.
    // Empty for a Mosquito that hasn't picked an imitation target yet:
    // there's nothing to move or throw as until imitating_ is set.
    std::vector<ActionKind> actionOptions() const override;

    // Every distinct `imitating` value among candidates_'s
    // MosquitoMovement entries, plus Pillbug if candidates_ has any
    // PillbugThrow entries -- empty for every non-Mosquito piece and
    // for an elevated Mosquito. Derived from game.legalMoves()
    // (candidates_), never from imitatableTypes() directly, so a type
    // that's adjacent but can't structurally move/throw is never
    // offered (see the plan's note on this).
    std::vector<PieceType> imitationOptions() const override;

    std::optional<PieceType> currentImitation() const override;

    // Always Idle -- deselecting a piece never needs any data (contrast
    // AwaitingVictim/AwaitingThrowDestination, whose cancel() has to
    // rebuild an ActionMenu from scratch).
    std::unique_ptr<InputState> cancel(Game &game) override;

private:
    Hex source_;
    std::vector<Move> candidates_;

    // Snapshotted once at construction, same as candidates_: whether
    // source_ itself is a Mosquito. imitationOptions() needs this
    // directly rather than inferring it from candidates_'s shape,
    // since a real Pillbug's own PillbugThrow entries are otherwise
    // indistinguishable from a Mosquito's borrowed ones (see
    // Game::addMosquitoMoves) -- without it, selecting a real Pillbug
    // would wrongly offer "imitate Pillbug" on itself.
    bool sourceIsMosquito_;

    // Nullopt until chosen, and always nullopt for every non-Mosquito
    // piece and for an elevated Mosquito (Beetle pattern, no imitation
    // choice -- see Game::addMosquitoMoves). Whether a Mosquito is
    // ground-level or elevated is never tracked separately here: it
    // falls out of candidates_'s own contents (see imitationOptions()),
    // so there's no separate piece-type field to keep in sync with it.
    std::optional<PieceType> imitating_;
};
